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

#include "cAVS/Linux/MockedDevice.hpp"
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

const int filehandler = 0xDEADBEEF;

/* This test case uses the mocked device with expected input, i.e. the mocking test is successful*/
TEST_CASE("MockedDevice: linux read/write TEST")
{
    int nbbytes;
    MockedDevice device([] {});

    /** Using the command below to add the test vectors to the mock driver */

    device.addDebugfsEntryOKOpen("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl");
    device.addDebugfsEntryOKWrite(write_buffer_01, write_buffer_01.size());
    device.addDebugfsEntryOKRead(read_buffer_01_ro, read_buffer_01_ro.size(),
                                 read_buffer_01_ro.size());
    device.addDebugfsEntryOKClose();

    /** Now using the mocked device */

    /** Open the file */
    CHECK_NOTHROW(device.debugfsOpen("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl"));
    /** write command */
    CHECK_NOTHROW(nbbytes = device.debugfsWrite(write_buffer_01));
    /** the number of bytes that been written should be the same as requested */
    CHECK(nbbytes == write_buffer_01.size());
    /** read reply */
    Buffer read_buffer_01_w;
    read_buffer_01_w.resize(read_buffer_01_ro.size());
    CHECK_NOTHROW(nbbytes = device.debugfsRead(read_buffer_01_w, read_buffer_01_w.size()));
    /** the number of bytes that been read should be the same as requested */
    CHECK(nbbytes == read_buffer_01_w.size());
    /** the number of bytes read should be the number of bytes requested */
    CHECK_NOTHROW(device.debugfsClose());
}

/* This test case uses the mocked device with falling input, mock device should return fail */
TEST_CASE("MockedDevice: linux read/write testing exception when falling")
{
    MockedDevice device([] {});

    /** add KO vector and check there is exceptions */
    SECTION ("OS Open error") {
        device.addDebugfsEntryKOOpen("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl");

        CHECK_THROWS_AS_MSG(device.debugfsOpen("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl"),
                            Device::Exception, "error during open: error#MockDevice");
    }

    SECTION ("OS Write error") {
        device.addDebugfsEntryKOWrite(write_buffer_01, write_buffer_01.size());

        CHECK_THROWS_AS_MSG(device.debugfsWrite(write_buffer_01), Device::Exception,
                            "error during write: error#MockDevice");
    }

    SECTION ("OS Read error") {
        device.addDebugfsEntryKORead(read_buffer_01_ro, read_buffer_01_ro.size(),
                                     read_buffer_01_ro.size());

        Buffer read_buffer_01_w;
        read_buffer_01_w.resize(read_buffer_01_ro.size());

        CHECK_THROWS_AS_MSG(device.debugfsRead(read_buffer_01_w, read_buffer_01_w.size()),
                            Device::Exception, "error during read: error#MockDevice");
    }

    SECTION ("OS Close error") {
        device.addDebugfsEntryKOClose();

        CHECK_THROWS_AS_MSG(device.debugfsClose(), Device::Exception,
                            "error during close: error#MockDevice");
    }

    SECTION ("Reach unexpected end of vector") {
        device.addDebugfsEntryOKOpen("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl");
        CHECK_NOTHROW(device.debugfsOpen("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl"));
        CHECK_THROWS_AS_MSG(device.debugfsOpen("/sys/kernel/debug/snd_soc_test/adsp_prop_ctrl"),
                            Device::Exception, "Mock failed: Debugfs vector already consumed.");
    }
}
