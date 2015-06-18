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

#include "cAVS/Windows/LastError.hpp"
#include <Windows.h>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

std::string LastError::get()
{
    std::string errorMessage;

    /* Getting last error code */
    DWORD error = GetLastError();
    LPVOID msgBuf;

    /* Getting system error message. This function allocates a string, which shall be released
     * with LocalFree function. */
    DWORD bufLen = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&msgBuf,
        0, NULL);

    if (bufLen) {
        /* Format message appends \r\n chars at the end of the string. Removing them */
        if (bufLen >= 2) {
            bufLen -= 2;
        }

        LPCSTR lpMsgStr = (LPCSTR)msgBuf;
        errorMessage = std::string(lpMsgStr, bufLen);
        LocalFree(msgBuf);
    }
    else {
        errorMessage = "Unable to get error message";
    }

    return "ERR " + std::to_string(error) + ": " + errorMessage;
}

}
}
}
