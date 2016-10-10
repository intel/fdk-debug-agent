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

namespace debug_agent
{
namespace cavs
{
namespace windows
{

std::string LastError::get()
{
    /* Getting last error code
     * Warning: do not put any code before the call to GetLastError(): we have observed that merely
     * declaring a string resets the last error code.
     */
    DWORD error = GetLastError();

    std::string errorMessage;
    LPVOID msgBuf;

    /* Getting system error message. This function allocates a string, which shall be released
     * with LocalFree function.
     *
     * FormatMessageA is the ansi version of the Windows FormatMessage method. It has to be called
     * explicitly because "FormatMessage" is a macro defined by
     * windows.h, which is then undefined by the POCO library. */

    DWORD bufLen = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuf, 0, NULL);

    if (bufLen) {
        /* Format message appends \r\n chars at the end of the string. Removing them */
        if (bufLen >= 2) {
            bufLen -= 2;
        }

        LPCSTR lpMsgStr = (LPCSTR)msgBuf;
        errorMessage = std::string(lpMsgStr, bufLen);
        LocalFree(msgBuf);
    } else {
        errorMessage = "Unable to get error message";
    }

    return "ERR " + std::to_string(error) + ": " + errorMessage;
}
}
}
}
