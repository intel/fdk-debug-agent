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
#include "cAVS/Windows/WindowsTypes.hpp"
#include <evntrace.h>
#include <mutex>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class pilots wpp real time log session.
 *
 * Produced events can be consumed using the WppConsumer class.
 *
 * Notes:
 * - when start() is called, if a previous session exists, it is stopped.
 * - this class is thread safe, start() and stop() can be called concurrently
 */
class WppController final
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    WppController();
    ~WppController();

    /* Start the session.
     *
     * if a previous session exists, it is stopped.
     * @throw WppController::Exception
     */
    void start();

    void stop() noexcept;

private:
    /* Values defined by
     * https://msdn.microsoft.com/en-us/library/windows/desktop/aa364160(v=vs.85).aspx
     *
     * (ClientContext member)
     */
    enum ClientContextValues
    {
        PerformanceCounter = 1,
        SystemTime = 2,
        CPUCycleCount = 3
    };

    /** Wpp requires a structure that contains an EVENT_TRACE_PROPERTIES instance followed by
     * the session name, then the log file name.
     *
     * @see https://msdn.microsoft.com/en-us/library/windows/desktop/aa364160(v=vs.85).aspx
     * (description of member "BufferSize")
     */
    struct TraceProperties
    {
        EVENT_TRACE_PROPERTIES properties;

        /**
        * from https://msdn.microsoft.com/en-us/library/windows/desktop/aa364103(v=vs.85).aspx:
        *
        * "You can use the maximum session name (1024 characters) and maximum log file name
        * (1024 characters) lengths to calculate the buffer size and offsets if not known."
        */
        static const std::size_t sessionNameMaxLen = 1024;
        static const std::size_t logFileNameMaxLen = 1024;

        char sessionName[sessionNameMaxLen];
        char logFileName[logFileNameMaxLen];

        TraceProperties();

        /* Set initial structure content:
         * The whole content is firstly set to zero, then the properties.Wnode.BufferSize field
         * is initialized.
         */
        void init();
    };

    WppController(const WppController &) = delete;
    WppController &operator=(const WppController &) = delete;

    /* If a previous session exists, delete it. */
    void cleanupOldSession();

    /* Must be called in a locked context. */
    void stopLocked();

    TRACEHANDLE mHandle;
    std::mutex mHandleMutex;
    TraceProperties mTraceProperties;
};
}
}
}
