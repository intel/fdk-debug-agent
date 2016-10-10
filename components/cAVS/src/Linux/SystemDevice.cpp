/*
 * Copyright (c) 2016, Intel Corporation
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

#include "cAVS/Linux/SystemDevice.hpp"
#include "cAVS/Linux/DriverTypes.hpp"

using namespace debug_agent::util;
using namespace std;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

ssize_t SystemDevice::commandWrite(const std::string &name, const util::Buffer &bufferInput)
{
    std::lock_guard<std::mutex> locker(mClientMutex);
    ssize_t written;
    try {
        mFileHandler->open(name);
    } catch (const FileEntryHandler::Exception &e) {
        throw Exception("DebugFs handler returns an exception: " + std::string(e.what()));
    }
    try {
        written = mFileHandler->write(bufferInput);
    } catch (const FileEntryHandler::Exception &e) {
        mFileHandler->close();
        throw Exception("Failed to write command in file: " + name +
                        ", DebugFs handler returns an exception: " + std::string(e.what()));
    }
    mFileHandler->close();
    return written;
}

void SystemDevice::commandRead(const std::string &name, const util::Buffer &bufferInput,
                               util::Buffer &bufferOutput)
{
    std::lock_guard<std::mutex> locker(mClientMutex);
    try {
        mFileHandler->open(name);
    } catch (FileEntryHandler::Exception &e) {
        throw Exception("DebugFs handler returns an exception: " + std::string(e.what()));
    }

    /* Performing the debugfs write command, size ignored, as exception raised if partial write. */
    try {
        mFileHandler->write(bufferInput);
    } catch (const FileEntryHandler::Exception &e) {
        mFileHandler->close();
        throw Exception("Failed to write command in file: " + name +
                        ", DebugFs handler returns an exception: " + std::string(e.what()));
    }

    /* Reading the result of debugfs command read, size ignored as not meaningful info. */
    try {
        mFileHandler->read(bufferOutput, bufferOutput.size());
    } catch (const FileEntryHandler::Exception &e) {
        mFileHandler->close();
        throw Exception("Failed to read command answer from file: " + name +
                        ", DebugFs handler returns an exception: " + std::string(e.what()));
    }
    mFileHandler->close();
}
}
}
}
