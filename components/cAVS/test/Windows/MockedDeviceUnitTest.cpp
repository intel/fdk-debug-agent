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

#include "cAVS/Windows/MockedDevice.hpp"
#include "TestCommon/TestHelpers.hpp"
#include <catch.hpp>
#include <memory>

using namespace debug_agent::cavs::windows;

/* Defining some ioctl codes */
static const uint32_t IoCtl1 = 1;
static const uint32_t IoCtl2 = 2;
static const uint32_t IoCtl3 = 3;
static const uint32_t IoCtl4 = 4;
static const uint32_t IoCtl5 = 5;

/* Defining some ioctl structure */
struct IoCtl_input
{
    IoCtl_input(uint32_t param1) : p1(param1) {}
    uint32_t p1;
};

bool operator == (const IoCtl_input &v1, const IoCtl_input &v2)
{
    return v1.p1 == v2.p1;
}

struct IoCtl_Output
{
    IoCtl_Output(uint32_t param1, uint32_t param2) : p1(param1), p2(param2) {}
    uint32_t p1;
    uint32_t p2;
};

bool operator == (const IoCtl_Output &v1, const IoCtl_Output &v2)
{
    return v1.p1 == v2.p1 && v1.p2 == v2.p2;
}

/* This test case uses the mocked device with expected input, i.e. the mocking is successful*/
TEST_CASE("MockedDevice: expected inputs")
{
    TypedBuffer<IoCtl_input> expectedInput(IoCtl_input(1));
    TypedBuffer<IoCtl_Output> expectedOutput(IoCtl_Output(2, 3));
    TypedBuffer<IoCtl_Output> returnedOutput(IoCtl_Output(5, 6));

    MockedDevice device;

    /* Setting the test vector
     * ----------------------- */

    /* All buffers are null */
    device.addIoctlEntry(IoCtl1, nullptr, nullptr, nullptr, true);

    /* Only input buffer */
    device.addIoctlEntry(IoCtl2, &expectedInput, nullptr, nullptr, true);

    /* Only output buffer */
    device.addIoctlEntry(IoCtl3, nullptr, &expectedOutput, &returnedOutput, true);

    /* Both input and output buffers */
    device.addIoctlEntry(IoCtl4, &expectedInput, &expectedOutput, &returnedOutput, true);

    /* The iotcl should return failure (success parameter is set to false) */
    device.addIoctlEntry(IoCtl5, nullptr, nullptr, nullptr, false);

    /* Now using the mocked device
     * --------------------------- */

    /* All buffers are null */
    CHECK_NOTHROW(device.ioControl(IoCtl1, nullptr, nullptr));

    /* Only input buffer */
    CHECK_NOTHROW(device.ioControl(IoCtl2, &expectedInput, nullptr));

    /* Only output buffer */
    TypedBuffer<IoCtl_Output> output1(expectedOutput);
    CHECK_NOTHROW(device.ioControl(IoCtl3, nullptr, &output1));
    CHECK(output1 == returnedOutput);

    /* Both input and output buffers */
    TypedBuffer<IoCtl_Output> output2(expectedOutput);
    CHECK_NOTHROW(device.ioControl(IoCtl4, &expectedInput, &output2));
    CHECK(output2 == returnedOutput);

    /* The iotcl should return failure */
    CHECK_THROWS_MSG(device.ioControl(IoCtl5, nullptr, nullptr),
        "Mock specifies failure.");

    CHECK_NOTHROW(device.checkMockingSuccess());
}

/* This test case uses the mocked device with unexpected inputs, i.e. the mocking has failed */
TEST_CASE("MockedDevice: unexpected inputs")
{
    TypedBuffer<IoCtl_input> expectedInput(IoCtl_input(1));
    TypedBuffer<IoCtl_input> unexpectedInput(IoCtl_input(9));

    TypedBuffer<IoCtl_Output> expectedOutput(IoCtl_Output(2, 3));
    TypedBuffer<IoCtl_Output> unexpectedOutput(IoCtl_Output(8, 10));
    TypedBuffer<IoCtl_Output> returnedOutput(IoCtl_Output(5, 6));

    MockedDevice device;
    SECTION("Test vector consumed") {
        device.addIoctlEntry(IoCtl1, nullptr, nullptr, nullptr, true);

        /* First ioctl: ok */
        CHECK_NOTHROW(device.ioControl(IoCtl1, nullptr, nullptr));

        /* Second ioctl: test vector is consumed */
        CHECK_THROWS_MSG(device.ioControl(IoCtl1, nullptr, nullptr),
            "Mock failed: IoCtl vector already consumed.");
    }

    SECTION("Wrong ioctl") {
        device.addIoctlEntry(IoCtl1, nullptr, nullptr, nullptr, true);

        CHECK_THROWS_MSG(device.ioControl(IoCtl2, nullptr, nullptr),
            "Mock failed: IOCtl entry #0: IoCtrl code: 2 expected : 1");

        CHECK_THROWS(device.checkMockingSuccess());
    }

    SECTION("Wrong input buffer content") {
        device.addIoctlEntry(IoCtl2, &expectedInput, nullptr, nullptr, true);

        CHECK_THROWS_MSG(device.ioControl(IoCtl2, &unexpectedInput, nullptr),
            "Mock failed: IOCtl entry #0: Input buffer content is not the expected one.");
    }

    SECTION("Wrong input buffer: input buffer should be null") {
        device.addIoctlEntry(IoCtl2, nullptr, nullptr, nullptr, true);

        CHECK_THROWS_MSG(device.ioControl(IoCtl2, &unexpectedInput, nullptr),
            "Mock failed: IOCtl entry #0: Input buffer should be null.");
    }

    SECTION("Wrong input buffer: input buffer should not be null") {
        device.addIoctlEntry(IoCtl2, &expectedInput, nullptr, nullptr, true);

        CHECK_THROWS_MSG(device.ioControl(IoCtl2, nullptr, nullptr),
            "Mock failed: IOCtl entry #0: Input buffer should not be null.");
    }

    SECTION("Wrong output buffer: output buffer should be null") {
        device.addIoctlEntry(IoCtl2, nullptr, nullptr, nullptr, true);

        TypedBuffer<IoCtl_Output> output(unexpectedOutput);
        CHECK_THROWS_MSG(device.ioControl(IoCtl2, nullptr, &output),
            "Mock failed: IOCtl entry #0: Output buffer should be null.");
    }

    SECTION("Wrong output buffer: output buffer should not be null") {
        device.addIoctlEntry(IoCtl2, nullptr, &expectedOutput, &returnedOutput, true);

        CHECK_THROWS_MSG(device.ioControl(IoCtl2, nullptr, nullptr),
            "Mock failed: IOCtl entry #0: Output buffer should not be null.");
    }

    SECTION("Wrong output buffer: output buffer content is not the expected one.") {
        device.addIoctlEntry(IoCtl2, nullptr, &expectedOutput, &returnedOutput, true);

        TypedBuffer<IoCtl_Output> output(unexpectedOutput);
        CHECK_THROWS_MSG(device.ioControl(IoCtl2, nullptr, &output),
            "Mock failed: IOCtl entry #0: Output buffer content is not the expected one.");
    }

    SECTION("Wrong output buffer: output buffer too small") {
        device.addIoctlEntry(IoCtl2, nullptr, &expectedOutput, &returnedOutput, true);

        struct TooSmallOutput {
            uint32_t a;
        };

        TypedBuffer<TooSmallOutput> tooSmallOutput;

        CHECK_THROWS_MSG(device.ioControl(IoCtl2, nullptr, &tooSmallOutput),
            "Mock failed: IOCtl entry #0: Candidate output buffer size 4 differs "
            "from required size: 8");
    }

    SECTION("Test vector not fully consumed") {
        device.addIoctlEntry(IoCtl2, nullptr, nullptr, nullptr, true);
        device.addIoctlEntry(IoCtl1, nullptr, nullptr, nullptr, true);

        CHECK_NOTHROW(device.ioControl(IoCtl2, nullptr, nullptr));

        CHECK_THROWS_MSG(device.checkMockingSuccess(),
            "IoCtl test vector has not been fully consumed.");
    }
}


