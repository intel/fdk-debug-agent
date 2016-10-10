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

#include "cAVS/Windows/LastError.hpp"
#include "cAVS/Windows/WindowsTypes.hpp"
#include <catch.hpp>
#include <string>

using namespace debug_agent::cavs::windows;

class SafeHandle final
{
public:
    SafeHandle(HANDLE handle) : mHandle(handle) {}

    ~SafeHandle()
    {
        if (isValid()) {
            CloseHandle(mHandle);
        }
    }

    bool isValid() { return mHandle != INVALID_HANDLE_VALUE; }

private:
    SafeHandle(const SafeHandle &) = delete;
    SafeHandle &operator=(const SafeHandle &) = delete;
    HANDLE mHandle;
};

TEST_CASE("LastError")
{
    /** CreateFileA is the ansi version of the Windows CreateFile method. It has to be called
      * explicitly because "CreateFile" is a macro defined by windows.h,
      * which is then undefined by the POCO library. */
    HANDLE handle = CreateFileA("c:\\Unexisting_file", GENERIC_READ, 0, NULL, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, NULL);
    auto error = LastError::get();

    SafeHandle safeHandle(handle);

    /* File handle shall be invalid */
    REQUIRE(!safeHandle.isValid());

    /* Checking last error string */
    CHECK(error == "ERR 2: The system cannot find the file specified.");
}
