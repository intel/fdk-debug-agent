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

#include "cAVS/Linux/DebugFsEntryHandler.hpp"

using namespace debug_agent::util;
using namespace std;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

void DebugFsEntryHandler::open(const std::string &name)
{
    mFileEntry.exceptions(fstream::failbit | fstream::badbit | fstream::eofbit);
    if (mFileEntry.is_open()) {
        throw Exception("Parallel operation not permitted on same debugfs entry: " + name);
    }
    try {
        mFileEntry.open(name, std::fstream::in | std::fstream::out);
    } catch (const std::exception &) {
        throw Exception("error while opening debugfs " + name + " file.");
    }
}

void DebugFsEntryHandler::close() noexcept
{
    try {
        mFileEntry.close();
    } catch (const std::exception &) {
        mFileEntry.clear();
    }
}

ssize_t DebugFsEntryHandler::write(const Buffer &bufferInput)
{
    if (!mFileEntry.is_open()) {
        throw Exception("Illegal write operation on closed file");
    }
    try {
        mFileEntry.write(reinterpret_cast<const char *>(bufferInput.data()), bufferInput.size());
    } catch (const std::exception &) {
        throw Exception("error during write operation: " + mFileEntry.rdstate());
    }
    return bufferInput.size();
}

ssize_t DebugFsEntryHandler::read(Buffer &bufferOutput, const ssize_t nbBytes)
{
    if (!mFileEntry.is_open()) {
        throw Exception("Illegal read operation on closed file");
    }
    try {
        mFileEntry.seekg(0, ios_base::beg);
        mFileEntry.read(reinterpret_cast<char *>(bufferOutput.data()), nbBytes);
    } catch (const std::exception &) {
        if (mFileEntry.eof() && mFileEntry.gcount() > 0) {
            // Reading less than expected is not an error, as blind request with max size are ok.
            mFileEntry.clear();
        } else {
            throw Exception("error during read operation: " + mFileEntry.rdstate());
        }
    }
    return mFileEntry.gcount();
}
}
}
}
