/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
*   The source code contained  or  described herein and all documents related to
*   the source code ("Material") are owned by Intel Corporation or its suppliers
*   or licensors.  Title to the  Material remains with  Intel Corporation or its
*   suppliers and licensors. The Material contains trade secrets and proprietary
*   and  confidential  information of  Intel or its suppliers and licensors. The
*   Material  is  protected  by  worldwide  copyright  and trade secret laws and
*   treaty  provisions. No part of the Material may be used, copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way without Intel's prior express written permission.
*   No license  under any  patent, copyright, trade secret or other intellectual
*   property right is granted to or conferred upon you by disclosure or delivery
*   of the Materials,  either expressly, by implication, inducement, estoppel or
*   otherwise.  Any  license  under  such  intellectual property  rights must be
*   express and approved by Intel in writing.
*
********************************************************************************
*/
#include "cAVS/Windows/WppController.hpp"
#include "cAVS/Windows/LastError.hpp"
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

using namespace common_wpp_types;
using namespace controller_wpp_types;

WppController::TraceProperties::TraceProperties()
{
    init();
}

void WppController::TraceProperties::init()
{

    /* When memset'ing to 0 the whole structure, KW complains of a potential
     * issue because it is not POD. To fix it, only the relevant fields of
     * this structure are initialized.
     */
    memset(&properties, 0, sizeof(properties));
    memset(sessionName, 0, sizeof(sessionName));
    memset(logFileName, 0, sizeof(logFileName));

    WNODE_HEADER &header = properties.Wnode;

    /* from https://msdn.microsoft.com/en-us/library/windows/desktop/aa364160(v=vs.85).aspx:
    *
    * "BufferSize: Total size of memory allocated, in bytes, for the event tracing session
    * properties. The size of memory must include the room for the EVENT_TRACE_PROPERTIES
    * structure plus the session name string and log file name string that follow the structure
    * in memory."
    *
    * The TraceProperties structure (defined by us) contains EVENT_TRACE_PROPERTIES + session name
    * + log file name;
    */
    header.BufferSize = sizeof(TraceProperties);
}

WppController::WppController() : mHandle(INVALID_PROCESSTRACE_HANDLE)
{
}

WppController::~WppController()
{
    /* Ensure that the session is stopped */
    stop();
}

void WppController::start()
{
    std::lock_guard<std::mutex> locker(mHandleMutex);

    if (mHandle != INVALID_PROCESSTRACE_HANDLE) {
        /* Already started */
        return;
    }

    cleanupOldSession();

    /* Reset session properties */
    mTraceProperties.init();

    /* Setting session name */
    strcpy_s(mTraceProperties.sessionName, sizeof(mTraceProperties.sessionName), wppSessionName);

    /* Don't set logFileName member because it is not used, and it has already set to 0
     * within the TraceProperties constructor. */

    EVENT_TRACE_PROPERTIES &properties = mTraceProperties.properties;

    /* Note: There is no need to set the header.guid field:
     * from https://msdn.microsoft.com/en-us/library/windows/desktop/aa364160(v=vs.85).aspx:
     *
     * "If you start a session that is not a kernel logger or private logger session, you do not
     * have to specify a session GUID. If you do not specify a GUID, ETW creates one for you.
     * You need to specify a session GUID only if you want to change the default permissions
     * associated with a specific session. For details, see the EventAccessControl function.
     */

    /* Using the PerformanceCounter value in order to obtain high-resolution timestamps */
    properties.Wnode.ClientContext = ClientContextValues::PerformanceCounter;

    /* from https://msdn.microsoft.com/en-us/library/windows/desktop/aa364160(v=vs.85).aspx:
     *
     * "Must contain WNODE_FLAG_TRACED_GUID to indicate that the structure contains event tracing
     * information."
     */
    properties.Wnode.Flags = WNODE_FLAG_TRACED_GUID;

    /*
     * EVENT_TRACE_REAL_TIME_MODE: Delivers the events to consumers in real-time. Events are
     * delivered when the buffers are flushed, not at the time the provider writes the event.
     *
     * EVENT_TRACE_USE_LOCAL_SEQUENCE: Uses sequence numbers that are unique only for an individual
     * event tracing session.
     *
     * EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING: Writes events that were logged on different
     * processors to a common buffer. Using this mode can eliminate the issue of events appearing
     * out of order when events are being published on different processors using system time.
     * This mode can also eliminate the issue with circular logs appearing to drop events on
     * multiple processor computers.
     */
    properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_USE_LOCAL_SEQUENCE |
                             EVENT_TRACE_NO_PER_PROCESSOR_BUFFERING;

    /* LoggerNameOffset: Offset from the start of the structure's allocated memory to beginning
     * of the null-terminated string that contains the session name.*/
    properties.LoggerNameOffset =
        static_cast<ULONG>(reinterpret_cast<uint8_t *>(&mTraceProperties.logFileName) -
                           reinterpret_cast<uint8_t *>(&mTraceProperties));

    /* LogFileNameOffset: Offset from the start of the structure's allocated memory to beginning
     * of the null-terminated string that contains the log file name.
     *
     * If you do not want to log events to a log file (for example, you specify
     * EVENT_TRACE_REAL_TIME_MODE only), set LogFileNameOffset to 0.
     */
    properties.LogFileNameOffset = 0;

    /* Starting the session */
    ULONG status = StartTrace(&mHandle, wppSessionName, &properties);
    if (status != ERROR_SUCCESS) {
        mHandle = INVALID_PROCESSTRACE_HANDLE;
        throw Exception("Unable to start session: err=" + std::to_string(status));
    }

    /* Enabling the audio dsp log provider */
    status = EnableTrace(TRUE,                /* Enable provider */
                         fwLogFlag,           /* filter firmware logs */
                         TRACE_LEVEL_VERBOSE, /* retrieve all logs */
                         &AudioDspLogControlGuid, mHandle);

    if (ERROR_SUCCESS != status) {
        stopLocked();
        throw Exception("Unable to enable log provider: err=" + std::to_string(status));
    }
}

void WppController::stopLocked()
{
    /* Must be called in a locked context. */

    if (mHandle == INVALID_PROCESSTRACE_HANDLE) {
        /* Already stopped */
        return;
    }

    ULONG status = StopTrace(mHandle, NULL, &mTraceProperties.properties);
    if (status != ERROR_SUCCESS) {
        /* Not throwing exception because nothing can be done here except logging... */
        std::cout << "Unable to stop session: err=" << status << std::endl;
    }
    mHandle = INVALID_PROCESSTRACE_HANDLE;
}

void WppController::stop() noexcept
{
    std::lock_guard<std::mutex> locker(mHandleMutex);

    stopLocked();
}

void WppController::cleanupOldSession()
{
    TraceProperties traceProperties;

    /** Looking for an existing session */
    ULONG status = QueryTraceA(NULL, wppSessionName, &traceProperties.properties);
    if (status == ERROR_SUCCESS) {

        /* Session exists, stopping it */
        status = StopTraceA(NULL, wppSessionName, &traceProperties.properties);
        if (status != ERROR_SUCCESS) {
            throw Exception("Unable to stop existing session: err=" + std::to_string(status));
        }
    } else if (status != ERROR_WMI_INSTANCE_NOT_FOUND) {
        throw Exception("Unable to query existing session: err=" + std::to_string(status));
    }
}
}
}
}
