/*
 * Copyright (c) 2015, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
