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
#include "cAVS/Linux/MockedDebugFsEntryHandler.hpp"
#include "TestCommon/TestHelpers.hpp"
#include "Util/Buffer.hpp"
#include "Util/TypedBuffer.hpp"
#include <catch.hpp>
#include <memory>
#include <stdio.h>

#define NB_EL(x) (sizeof(x) / sizeof((x)[0]))

using namespace debug_agent::cavs::linux;
using namespace debug_agent::util;

const Buffer read_buffer_01_ro{5, 1, 2, 3, 4, 5};
const Buffer write_buffer_01{7, 1, 2, 3, 4, 5, 6, 7, 64};
const Buffer read_buffer_02{4, 1, 2, 3, 4, 5};
const Buffer write_buffer_02{7, 1, 2, 3, 4, 5, 6, 7};

static const std::string debugFsEntryFileName{"/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl"};

struct Fixture
{
    std::unique_ptr<MockedDebugFsEntryHandler> mockedDebugFsEntryHandler =
        std::make_unique<MockedDebugFsEntryHandler>([] {
            INFO("There are leftover test inputs");
            CHECK(false);
        });
};

/* This test case uses the mocked device with expected input, i.e. the mocking test is successful*/
TEST_CASE_METHOD(Fixture, "SystemDevice: read command TEST")
{
    /** Using the command below to add the test vectors to the mock driver */
    mockedDebugFsEntryHandler->addDebugfsEntryOKOpen(debugFsEntryFileName);
    mockedDebugFsEntryHandler->addDebugfsEntryOKWrite(write_buffer_01, write_buffer_01.size());
    mockedDebugFsEntryHandler->addDebugfsEntryOKRead(read_buffer_01_ro, 2048,
                                                     read_buffer_01_ro.size());
    mockedDebugFsEntryHandler->addDebugfsEntryOKClose();

    SystemDevice device(std::move(mockedDebugFsEntryHandler));
    /** Now using the system device injecting the mocked DebugFsEntryHandler.*/

    Buffer read_buffer_01_w;
    read_buffer_01_w.resize(2048);

    /** Reading Command */
    CHECK_NOTHROW(device.commandRead("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl",
                                     write_buffer_01, read_buffer_01_w));

    /** the number of bytes that been read should be the same as requested */
    CHECK(read_buffer_01_ro.size() == read_buffer_01_w.size());
    /** the number of bytes read should be the number of bytes requested */
}

TEST_CASE_METHOD(Fixture, "SystemDevice: write command TEST")
{
    /** Using the command below to add the test vectors to the mock driver */
    mockedDebugFsEntryHandler->addDebugfsEntryOKOpen(debugFsEntryFileName);
    mockedDebugFsEntryHandler->addDebugfsEntryOKWrite(write_buffer_01, write_buffer_01.size());
    mockedDebugFsEntryHandler->addDebugfsEntryOKClose();

    SystemDevice device(std::move(mockedDebugFsEntryHandler));
    /** Now using the system device injecting the mocked DebugFsEntryHandler.*/

    Buffer read_buffer_01_w;
    read_buffer_01_w.resize(read_buffer_01_ro.size());

    /** Command Write*/
    CHECK_NOTHROW(
        device.commandWrite("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl", write_buffer_01));
}

/* This test case uses the mocked device with falling input, mock device should return fail */
TEST_CASE_METHOD(Fixture, "SystemDevice: linux read/write testing exception when failing")
{
    /** add KO vector and check there is exceptions */
    SECTION ("OS Open error") {
        mockedDebugFsEntryHandler->addDebugfsEntryKOOpen(debugFsEntryFileName);

        SystemDevice device(std::move(mockedDebugFsEntryHandler));
        CHECK_THROWS_AS_MSG(device.commandWrite(debugFsEntryFileName, write_buffer_01),
                            Device::Exception, "DebugFs handler returns an exception: error during "
                                               "open: error#MockDebugFsEntryHandler");
    }

    SECTION ("OS Write error") {
        mockedDebugFsEntryHandler->addDebugfsEntryOKOpen(
            "/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl");
        mockedDebugFsEntryHandler->addDebugfsEntryKOWrite(write_buffer_01, write_buffer_01.size());
        mockedDebugFsEntryHandler->addDebugfsEntryOKClose();

        SystemDevice device(std::move(mockedDebugFsEntryHandler));

        CHECK_THROWS_AS_MSG(device.commandWrite(debugFsEntryFileName, write_buffer_01),
                            Device::Exception,
                            "Failed to write command in file: " + debugFsEntryFileName +
                                ", "
                                "DebugFs handler returns an exception: error during write: "
                                "error#MockDebugFsEntryHandler");
    }

    SECTION ("OS Read error") {
        mockedDebugFsEntryHandler->addDebugfsEntryOKOpen(
            "/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl");
        mockedDebugFsEntryHandler->addDebugfsEntryOKWrite(read_buffer_01_ro,
                                                          read_buffer_01_ro.size());
        mockedDebugFsEntryHandler->addDebugfsEntryKORead(
            read_buffer_01_ro, read_buffer_01_ro.size(), read_buffer_01_ro.size());
        mockedDebugFsEntryHandler->addDebugfsEntryOKClose();

        SystemDevice device(std::move(mockedDebugFsEntryHandler));

        Buffer read_buffer_01_w;
        read_buffer_01_w.resize(read_buffer_01_ro.size());

        CHECK_THROWS_AS_MSG(
            device.commandRead(debugFsEntryFileName, read_buffer_01_ro, read_buffer_01_w),
            Device::Exception, "Failed to read command answer from file: " + debugFsEntryFileName +
                                   ", "
                                   "DebugFs handler returns an exception: error during read: "
                                   "error#MockDebugFsEntryHandler");
    }

    SECTION ("OS Close error") {
        mockedDebugFsEntryHandler->addDebugfsEntryOKOpen(debugFsEntryFileName);
        mockedDebugFsEntryHandler->addDebugfsEntryOKWrite(write_buffer_01, write_buffer_01.size());
        mockedDebugFsEntryHandler->addDebugfsEntryKOClose();

        SystemDevice device(std::move(mockedDebugFsEntryHandler));
        CHECK_NOTHROW(device.commandWrite(debugFsEntryFileName, write_buffer_01));
    }

    SECTION ("Reach unexpected end of vector") {
        mockedDebugFsEntryHandler->addDebugfsEntryOKOpen(debugFsEntryFileName);
        mockedDebugFsEntryHandler->addDebugfsEntryOKWrite(write_buffer_01, write_buffer_01.size());
        mockedDebugFsEntryHandler->addDebugfsEntryOKClose();
        SystemDevice device(std::move(mockedDebugFsEntryHandler));
        CHECK_NOTHROW(device.commandWrite(debugFsEntryFileName, write_buffer_01));
        CHECK_THROWS_AS_MSG(device.commandWrite(debugFsEntryFileName, write_buffer_01),
                            Device::Exception, "DebugFs handler returns an exception: Mock failed: "
                                               "Debugfs vector already consumed.");
    }
}
