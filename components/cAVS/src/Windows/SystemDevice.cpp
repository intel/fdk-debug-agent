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

#include "cAVS/Windows/SystemDevice.hpp"
#include "cAVS/Windows/LastError.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

SystemDevice::SystemDevice(const std::string &deviceId) :
    mDeviceHandle(INVALID_HANDLE_VALUE)
{
    /** CreateFileA is the ansi version of the Windows CreateFile method. It has to be called
     * explicitly because "CreateFile" is a macro defined by windows.h,
     * which is then undefined by the POCO library. */
    HANDLE handle =
        CreateFileA(deviceId.c_str(),
                   GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL,
                   NULL);

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
    DWORD returnedSizeUnused;

    LPVOID inputBufferPtr = nullptr;
    DWORD inputBufferSize = 0;
    if (input != nullptr) {
        /* Unfortunatley DeviceIoControl() input buffer parameter
        * is not const, so casting it from const to non const */
        inputBufferPtr = const_cast<LPVOID>(input->getPtr());
        inputBufferSize = static_cast<DWORD>(input->getSize());
    }

    LPVOID outputBufferPtr = nullptr;
    DWORD outputBufferSize = 0;
    if (output != nullptr) {
        outputBufferPtr = output->getPtr();
        outputBufferSize = static_cast<DWORD>(output->getSize());
    }

    BOOL result = DeviceIoControl(
        mDeviceHandle,
        ioControlCode,
        inputBufferPtr,
        inputBufferSize,
        outputBufferPtr,
        outputBufferSize,
        &returnedSizeUnused,
        NULL);

    if (result == FALSE) {
        throw Exception("IOControl failure: " + LastError::get());
    }
}

}
}
}
