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
#include "cAVS/Windows/WppConsumer.hpp"
#include <evntcons.h>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

using namespace common_wpp_types;
using namespace consumer_wpp_types;

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
    FwLogEntry &entry = *static_cast<FwLogEntry *>(pEvent->UserData);

    /* Checking that buffer size returned by the driver and buffer size returned by wpp are the
     * same. */
    if (entry.size != entry.wppBufferSize) {
        std::cout << "Warning: entry size " << entry.size << " differs from wpp size "
                  << entry.wppBufferSize << std::endl;
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
    if (pEvent->UserDataLength < (sizeof(FwLogEntry) - 1) + entry.size) {
        std::cout << "Warning: the wpp event size doesn't fit the fw log buffer size" << std::endl;
        return;
    }

    /* Checking that the user context is not null */
    if (pEvent->UserContext == nullptr) {
        std::cout << "Warning: log user context is null." << std::endl;
        return;
    }

    /* And finally notifying the listener... */
    WppLogEntryListener &listener = *static_cast<WppLogEntryListener *>(pEvent->UserContext);
    listener.onLogEntry(entry.coreId, entry.buffer, entry.size);
}

void WppConsumer::collectLogEntries(WppLogEntryListener &listener, const std::string &fileName)
{
    /* Setting structure memory content to 0 */
    EVENT_TRACE_LOGFILE traceInfo;
    SecureZeroMemory(&traceInfo, sizeof(traceInfo));

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
        traceInfo.LoggerName = const_cast<char *>(wppSessionName);

        /* Adding PROCESS_TRACE_MODE_REAL_TIME trace mode:
         *
         * PROCESS_TRACE_MODE_REAL_TIME: Specify this mode to receive events in real time
         * (you must specify this mode if LoggerName is not NULL).
         */

        traceInfo.ProcessTraceMode |= PROCESS_TRACE_MODE_REAL_TIME;
    } else {
        /* Logging from file */

        /* Setting log file name
         * Unfortunately the EVENT_TRACE_LOGFILE structure doesn't use const for input members */
        traceInfo.LogFileName = const_cast<char *>(fileName.c_str());

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
    if (!handle.isValid()) {
        throw Exception("Unable to open consumer trace session.");
    }

    /* Looping on log entries until termination. */
    ULONG status = ProcessTrace(&handle.get(), 1, /* handle count, one in our case */
                                0,                /* start time, not used */
                                0);               /* end time, not used */

    if (status != ERROR_SUCCESS) {
        throw Exception("Unable to collect log entries: err=" + std::to_string(status));
    }
}
}
}
}
