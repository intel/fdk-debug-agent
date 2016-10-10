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
TEST_CASE("DebugFsEntryHandler: testing interface of the real DebugFsEntryHandler")
{
    DebugFsEntryHandler debugFsEntryHandler;
    int handlef = 0;
    ssize_t nbbytes = 0;
    const size_t sizeBuffRead = 10;
    Buffer bufferread(sizeBuffRead, 0xff);

    /** For test purpose, need to create a temporary file */
    char name1[] = "./tmpDbgaSystemDeviceUnitTestXXXXXX";
    int tmpFd = mkstemp(name1);
    CHECK(tmpFd != -1);
    std::cout << "filename is " << name1;
    handlef = open(name1, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    CHECK(write(handlef, (void *)"0123456789012345", sizeBuffRead) == sizeBuffRead);
    close(handlef);

    /** real testing of the interface */
    CHECK_NOTHROW(debugFsEntryHandler.open(name1));
    /** then check the handle is correct (non nul) */
    /** no exception when read file */
    CHECK_NOTHROW(nbbytes = debugFsEntryHandler.read(bufferread, sizeBuffRead));
    /** the number of bytes that been written should be the same as requested */
    CHECK(nbbytes == sizeBuffRead);
    /** the number of bytes read should be the number of bytes requested */
    CHECK_NOTHROW(debugFsEntryHandler.close());

    /** for test purpose, removing temporary file */
    unlink(name1);

    /** now the file is erased, we should get error exception */
    CHECK_THROWS_AS_MSG(debugFsEntryHandler.open(name1), FileEntryHandler::Exception,
                        "error while opening debugfs " + std::string(name1) + " file.");
}
