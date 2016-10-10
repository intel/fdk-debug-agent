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
#include "cAVS/Windows/FileWppClient.hpp"
#include "cAVS/Windows/WppConsumer.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

FileWppClient::FileWppClient(const std::string &fileName) : mFileName(fileName), mStopRequest(false)
{
    if (mFileName.empty()) {
        throw Exception("FileWppClient: supplied log file name is empty.");
    }
}

void FileWppClient::collectLogEntries(WppLogEntryListener &listener)
{
    {
        std::lock_guard<std::mutex> locker(mStopRequestMutex);
        if (mStopRequest) {
            return;
        }
    }

    try {
        WppConsumer::collectLogEntries(listener, mFileName);
    } catch (WppConsumer::Exception &e) {
        throw Exception("Cannot collect entries with wpp consumer: " + std::string(e.what()));
    }
}

void FileWppClient::stop() noexcept
{
    std::lock_guard<std::mutex> locker(mStopRequestMutex);
    mStopRequest = true;

    /* If collectLogEntries() is running, it will not be interrupted by stop(), there is no way to
     * do that properly when reading entries from a log file.
     *
     * I have tried CloseTrace(mHandle) but its changes nothing because all entries have already
     * been put into the driver buffer.
     *
     * However reading entries from a log file is nearly instantaneous, therefore it does not
     * matter than stop() does not interrupt collectLogEntries() immediately.
     */
}
}
}
}
