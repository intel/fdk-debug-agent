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
#include "cAVS/Windows/WppConsumer.hpp"
#include <evntcons.h>


namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** GUID of the log provider */
static const GUID AudioDspProviderGuid =
{ 0xDB264037, 0x6BA1, 0x4DC0, { 0xAE, 0x16, 0x5C, 0x60, 0xAD, 0x47, 0x0E, 0xDD } };


VOID WINAPI WppConsumer::ProcessWppEvent(PEVENT_RECORD pEvent)
{
    if (pEvent == nullptr) {
        /* This should not happen. */
        std::cout << "Receiving null event record... " << std::endl;
        return;
    }

    EVENT_HEADER &header = pEvent->EventHeader;

    /* Excluding wrong GUID */
    if (!IsEqualGUID(header.ProviderId, AudioDspProviderGuid)) {
        return;
    }

    /* Excluding wrong descriptor id */
    if (header.EventDescriptor.Id != fwLogEventDescriptorId) {
        return;
    }

    /* Checking userdata nullness */
    if (pEvent->UserData == nullptr) {
        std::cout << "Warning: log entry user data is null." << std::endl;
        return;
    }

    /* Checking userdata size before casting it in order to avoid memory access violation when
     * using struct members */
    if (pEvent->UserDataLength < sizeof(FwLogEntry)) {
        std::cout << "Warning: log entry user data size is too small." << std::endl;
        return;
    }

    /* Now we can safely cast it */
    FwLogEntry &entry = *static_cast<FwLogEntry*>(pEvent->UserData);

    /* Checking that buffer size returned by the driver and buffer size returned by wpp are the
     * same. */
    if (entry.size != entry.wppBufferSize) {
        std::cout << "Warning: entry size " << entry.size << " differs from wpp size " <<
            entry.wppBufferSize << std::endl;
        return;
    }

    /* Checking that the fw log buffer size fits the wpp event size, .i.e:
     * whole wpp event size >= FwLogEntry struct size  + fw log buffer size
     *
     * In this way it is ensured that no memory access violation will occur when reading the
     * buffer content.
     *
     * Note: the FwLogEntry structure contains already one buffer byte because the declaration is:
     * struct FwLogEntry
     * {
     *     ...
     *     uint8_t buffer[1];
     * }
     *
     * Therefore the exact formula is:
     * whole wpp event size >= (FwLogEntry struct size - 1)  + fw log buffer size
     */
    if (pEvent->UserDataLength < (sizeof(FwLogEntry) - 1) + entry.size)
    {
        std::cout << "Warning: the wpp event size doesn't fit the fw log buffer size" << std::endl;
        return;
    }

    /* Checking that the user context is not null */
    if (pEvent->UserContext == nullptr) {
        std::cout << "Warning: log user context is null." << std::endl;
        return;
    }

    /* And finally notifying the listener... */
    WppLogEntryListener &listener = *static_cast<WppLogEntryListener*>(pEvent->UserContext);
    listener.onLogEntry(entry.coreId, entry.buffer, entry.size);
}

void WppConsumer::collectLogEntries(WppLogEntryListener &listener, const std::string &fileName)
{
    /* Setting structure memory content to 0 */
    EVENT_TRACE_LOGFILE traceInfo = { 0 };

    /* Setting trace mode
     *
     * PROCESS_TRACE_MODE_EVENT_RECORD: Specify this mode if you want to receive events in the
     * new EVENT_RECORD format.
     */
    traceInfo.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD;

    if (fileName.empty()) {
        /* Realtime logging */

        /* Log file name is not used*/
        traceInfo.LogFileName = nullptr;

        /* Setting session name
         * Unfortunately the EVENT_TRACE_LOGFILE structure doesn't use const for input members */
        traceInfo.LoggerName = const_cast<char*>(wppSessionName);

        /* Adding PROCESS_TRACE_MODE_REAL_TIME trace mode:
         *
         * PROCESS_TRACE_MODE_REAL_TIME: Specify this mode to receive events in real time
         * (you must specify this mode if LoggerName is not NULL).
         */

        traceInfo.ProcessTraceMode |= PROCESS_TRACE_MODE_REAL_TIME;
    }
    else {
        /* Logging from file */

        /* Setting log file name
         * Unfortunately the EVENT_TRACE_LOGFILE structure doesn't use const for input members */
        traceInfo.LogFileName = const_cast<char*>(fileName.c_str());

        /* Session name is not used */
        traceInfo.LoggerName = nullptr;
    }

    /* Setting callback */
    traceInfo.EventRecordCallback = ProcessWppEvent;

    /* Setting user context : the listener that will be used in the callback */
    traceInfo.Context = &listener;

    /* Opening the consumer session */
    SafeTraceHandler handle;
    handle.get() = OpenTrace(&traceInfo);
    if (!handle.isValid())
    {
        throw Exception("Unable to open consumer trace session.");
    }

    /* Looping on log entries until termination. */
    ULONG status = ProcessTrace(
        &handle.get(),
        1, /* handle count, one in our case */
        0, /* start time, not used */
        0); /* end time, not used */

    if (status != ERROR_SUCCESS) {
        throw Exception("Unable to collect log entries: err=" + std::to_string(status));
    }
}

}
}
}
