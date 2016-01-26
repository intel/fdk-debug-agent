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
#include "Util/Buffer.hpp"
#include "Util/TypedBuffer.hpp"
#include <catch.hpp>
#include <memory>

using namespace debug_agent::cavs::windows;
using namespace debug_agent::util;

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

bool operator==(const IoCtl_input &v1, const IoCtl_input &v2)
{
    return v1.p1 == v2.p1;
}

struct IoCtl_Output
{
    IoCtl_Output(uint32_t param1, uint32_t param2) : p1(param1), p2(param2) {}
    uint32_t p1;
    uint32_t p2;
};

bool operator==(const IoCtl_Output &v1, const IoCtl_Output &v2)
{
    return v1.p1 == v2.p1 && v1.p2 == v2.p2;
}

struct Fixture
{
    MockedDevice device;

    ~Fixture() { CHECK(device.consumed()); }
};

/* This test case uses the mocked device with expected input, i.e. the mocking is successful*/
TEST_CASE_METHOD(Fixture, "MockedDevice: expected inputs")
{
    TypedBuffer<IoCtl_input> expectedInput(IoCtl_input(1));
    TypedBuffer<IoCtl_Output> expectedOutput(IoCtl_Output(2, 3));
    TypedBuffer<IoCtl_Output> returnedOutput(IoCtl_Output(5, 6));

    /* Setting the test vector
     * ----------------------- */

    /* All buffers are null */
    device.addSuccessfulIoctlEntry(IoCtl1, nullptr, nullptr, nullptr);

    /* Only input buffer */
    device.addSuccessfulIoctlEntry(IoCtl2, &expectedInput, nullptr, nullptr);

    /* Only output buffer */
    device.addSuccessfulIoctlEntry(IoCtl3, nullptr, &expectedOutput, &returnedOutput);

    /* Both input and output buffers */
    device.addSuccessfulIoctlEntry(IoCtl4, &expectedInput, &expectedOutput, &returnedOutput);

    /* The iotcl should return failure (success parameter is set to false) */
    device.addFailedIoctlEntry(IoCtl5, nullptr, nullptr);

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
    CHECK_THROWS_AS_MSG(device.ioControl(IoCtl5, nullptr, nullptr), Device::Exception,
                        "OS says that io control has failed.");
}

/* This test case uses the mocked device with unexpected inputs, i.e. the mocking has failed */
TEST_CASE_METHOD(Fixture, "MockedDevice: unexpected inputs")
{
    TypedBuffer<IoCtl_input> expectedInput(IoCtl_input(1));
    TypedBuffer<IoCtl_input> unexpectedInput(IoCtl_input(9));

    TypedBuffer<IoCtl_Output> expectedOutput(IoCtl_Output(2, 3));
    TypedBuffer<IoCtl_Output> unexpectedOutput(IoCtl_Output(8, 10));
    TypedBuffer<IoCtl_Output> returnedOutput(IoCtl_Output(5, 6));

    SECTION ("Test vector consumed") {
        device.addSuccessfulIoctlEntry(IoCtl1, nullptr, nullptr, nullptr);

        /* First ioctl: ok */
        CHECK_NOTHROW(device.ioControl(IoCtl1, nullptr, nullptr));

        /* Second ioctl: test vector is consumed */
        CHECK_THROWS_AS_MSG(device.ioControl(IoCtl1, nullptr, nullptr), Device::Exception,
                            "Mock failed: IoCtl vector already consumed.");
    }

    SECTION ("Wrong ioctl") {
        device.addSuccessfulIoctlEntry(IoCtl1, nullptr, nullptr, nullptr);

        CHECK_THROWS_AS_MSG(device.ioControl(IoCtl2, nullptr, nullptr), Device::Exception,
                            "Mock failed: IOCtl entry #1: IoCtrl code: 2 expected : 1");
    }

    SECTION ("Wrong input buffer content") {
        device.addSuccessfulIoctlEntry(IoCtl2, &expectedInput, nullptr, nullptr);

        CHECK_THROWS_AS_MSG(
            device.ioControl(IoCtl2, &unexpectedInput, nullptr), Device::Exception,
            "Mock failed: IOCtl entry #1: Input buffer content is not the expected one.");
    }

    SECTION ("Wrong input buffer: input buffer should be null") {
        device.addSuccessfulIoctlEntry(IoCtl2, nullptr, nullptr, nullptr);

        CHECK_THROWS_AS_MSG(device.ioControl(IoCtl2, &unexpectedInput, nullptr), Device::Exception,
                            "Mock failed: IOCtl entry #1: Input buffer should be null.");
    }

    SECTION ("Wrong input buffer: input buffer should not be null") {
        device.addSuccessfulIoctlEntry(IoCtl2, &expectedInput, nullptr, nullptr);

        CHECK_THROWS_AS_MSG(device.ioControl(IoCtl2, nullptr, nullptr), Device::Exception,
                            "Mock failed: IOCtl entry #1: Input buffer should not be null.");
    }

    SECTION ("Wrong output buffer: output buffer should be null") {
        device.addSuccessfulIoctlEntry(IoCtl2, nullptr, nullptr, nullptr);

        TypedBuffer<IoCtl_Output> output(unexpectedOutput);
        CHECK_THROWS_AS_MSG(device.ioControl(IoCtl2, nullptr, &output), Device::Exception,
                            "Mock failed: IOCtl entry #1: Output buffer should be null.");
    }

    SECTION ("Wrong output buffer: output buffer should not be null") {
        device.addSuccessfulIoctlEntry(IoCtl2, nullptr, &expectedOutput, &returnedOutput);

        CHECK_THROWS_AS_MSG(device.ioControl(IoCtl2, nullptr, nullptr), Device::Exception,
                            "Mock failed: IOCtl entry #1: Output buffer should not be null.");
    }

    SECTION ("Wrong output buffer: output buffer content is not the expected one.") {
        device.addSuccessfulIoctlEntry(IoCtl2, nullptr, &expectedOutput, &returnedOutput);

        TypedBuffer<IoCtl_Output> output(unexpectedOutput);
        CHECK_THROWS_AS_MSG(
            device.ioControl(IoCtl2, nullptr, &output), Device::Exception,
            "Mock failed: IOCtl entry #1: Output buffer content is not the expected one.");
    }

    SECTION ("Wrong output buffer: output buffer too small") {
        device.addSuccessfulIoctlEntry(IoCtl2, nullptr, &expectedOutput, &returnedOutput);

        struct TooSmallOutput
        {
            uint32_t a;
        };

        TypedBuffer<TooSmallOutput> tooSmallOutput;

        CHECK_THROWS_AS_MSG(
            device.ioControl(IoCtl2, nullptr, &tooSmallOutput), Device::Exception,
            "Mock failed: IOCtl entry #1: Output buffer candidate with size 4 differs "
            "from required size: 8");
    }
}

TEST_CASE("MockedDevice: Test vector not fully consumed")
{
    MockedDevice device;
    device.addSuccessfulIoctlEntry(IoCtl2, nullptr, nullptr, nullptr);
    device.addSuccessfulIoctlEntry(IoCtl1, nullptr, nullptr, nullptr);

    CHECK_NOTHROW(device.ioControl(IoCtl2, nullptr, nullptr));
    CHECK_FALSE(device.consumed());
}
