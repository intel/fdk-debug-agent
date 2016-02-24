/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "cAVS/Linux/SystemDevice.hpp"
#include "Util/TypedBuffer.hpp"
#include "TestCommon/TestHelpers.hpp"
#include <catch.hpp>
#include <memory>
#include <iostream>
#include <cstdio>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace debug_agent::cavs::linux;
using namespace debug_agent::util;

/**
 * This functional test retrieves the baud rate of serial devices.
 * We assume that all machines have at least on serial device.
 */
TEST_CASE("SystemDevice: testing interface of the real device")
{
    SystemDevice systemDevice;
    int handlef = 0;
    ssize_t nbbytes = 0;
    const size_t sizeBuffRead = 10;
    uint8_t bufferread[sizeBuffRead];

    /** For test purpose, need to create a temporary file */
    char name1[] = "./tmpDbgaSystemDeviceUnitTestXXXXXX";
    int tmpFd = mkstemp(name1);
    CHECK(tmpFd != -1);
    std::cout << "filename is " << name1;
    handlef = open(name1, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    CHECK(write(handlef, (void *)"0123456789012345", sizeBuffRead) == sizeBuffRead);
    close(handlef);

    /** real testing of the interface */
    CHECK_NOTHROW(systemDevice.debugfsOpen(name1));
    /** then check the handle is correct (non nul) */
    /** no exception when read file */
    CHECK_NOTHROW(nbbytes = systemDevice.debugfsRead(bufferread, sizeBuffRead));
    /** the number of bytes that been written should be the same as requested */
    CHECK(nbbytes == sizeBuffRead);
    /** the number of bytes read should be the number of bytes requested */
    CHECK_NOTHROW(systemDevice.debugfsClose());

    /** for test purpose, removing temporary file */
    unlink(name1);

    /** now the file is erased, we should get error exception */
    CHECK_THROWS_AS_MSG(systemDevice.debugfsOpen(name1), Device::Exception,
                        "error while opening debugfs " + std::string(name1) + " file.");
}
