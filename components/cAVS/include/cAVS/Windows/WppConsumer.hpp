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
#pragma once

#include "cAVS/Windows/WppTypes.hpp"
#include "cAVS/Windows/WppLogEntryListener.hpp"
#include "cAVS/Windows/WindowsTypes.hpp"
#include <evntrace.h>
#include <string>
#include <inttypes.h>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class consumes wpp log entries
 *
 * The client has to implement the WppLogEntryListener interface in order to receive entries.
 *
 * Log events can be retrieved either from a realtime session or from a log file.
 */
class WppConsumer final
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /*
     * This method collects log entries, and reports them to the supplied listener.
     *
     * If a file name is supplied, logs are retrieved from a file. Otherwise logs are retrieved
     * from a realtime session.
     *
     * About the blocking behaviour:
     * - realtime session case: this method blocks until the realtime session ends.
     * - log file case: this method blocks until the log file is fully read.
     *
     * @throw WppConsumer::Exception */
    static void collectLogEntries(WppLogEntryListener &listener, const std::string &fileName = "");

private:
    /** This class ensures trace handle deletion */
    class SafeTraceHandler final
    {
    public:
        SafeTraceHandler() : mHandle(INVALID_PROCESSTRACE_HANDLE) {}

        ~SafeTraceHandler() { close(); }

        TRACEHANDLE &get() { return mHandle; }

        bool isValid() { return mHandle != INVALID_PROCESSTRACE_HANDLE; }

        void close()
        {
            if (isValid()) {
                ULONG status = CloseTrace(mHandle);
                if (status != ERROR_SUCCESS) {
                    /* @todo: use logging */
                    std::cout << "Unable to close trace handle: err=" << status << std::endl;
                }
            }
        }

    private:
        SafeTraceHandler(const SafeTraceHandler &) = delete;
        SafeTraceHandler &operator=(const SafeTraceHandler &) = delete;

        TRACEHANDLE mHandle;
    };

    /** Callback called by wpp when an event is received */
    static VOID WINAPI ProcessWppEvent(PEVENT_RECORD pEvent);
};
}
}
}
