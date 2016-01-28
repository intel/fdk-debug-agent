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
