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

#include "cAVS/Windows/SystemDevice.hpp"
#include "cAVS/Windows/LastError.hpp"

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace windows
{

SystemDevice::SystemDevice(const std::string &deviceId) : mDeviceHandle(INVALID_HANDLE_VALUE)
{
    /** CreateFileA is the ansi version of the Windows CreateFile method. It has to be called
     * explicitly because "CreateFile" is a macro defined by windows.h,
     * which is then undefined by the POCO library. */
    HANDLE handle = CreateFileA(deviceId.c_str(), GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, NULL);

    if (handle == INVALID_HANDLE_VALUE) {
        throw Exception("Can not open device " + deviceId + " : " + LastError::get());
    }

    mDeviceHandle = handle;
}

SystemDevice::~SystemDevice()
{
    if (mDeviceHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(mDeviceHandle);
    }
}

void SystemDevice::ioControl(uint32_t ioControlCode, const Buffer *input, Buffer *output)
{
    /* Generally the return size is not used. */
    DWORD returnedSize;

    LPVOID inputBufferPtr = nullptr;
    DWORD inputBufferSize = 0;
    if (input != nullptr) {
        /* Unfortunatley DeviceIoControl() input buffer parameter
        * is not const, so casting it from const to non const */
        inputBufferPtr = const_cast<LPVOID>(reinterpret_cast<LPCVOID>(input->data()));
        inputBufferSize = static_cast<DWORD>(input->size());
    }

    LPVOID outputBufferPtr = nullptr;
    DWORD outputBufferSize = 0;
    if (output != nullptr) {
        outputBufferPtr = output->data();
        outputBufferSize = static_cast<DWORD>(output->size());
    }

    BOOL result = DeviceIoControl(mDeviceHandle, ioControlCode, inputBufferPtr, inputBufferSize,
                                  outputBufferPtr, outputBufferSize, &returnedSize, NULL);

    if (result == FALSE) {
        throw Exception("IOControl failure: " + LastError::get());
    }

    if (output != nullptr) {

        if (returnedSize > output->size()) {
            throw Exception("IOControl response larger than buffer (" +
                            std::to_string(returnedSize) + " while output buffer is " +
                            std::to_string(output->size()) + ")");
        }

        output->resize(returnedSize);
    }
}
}
}
}
