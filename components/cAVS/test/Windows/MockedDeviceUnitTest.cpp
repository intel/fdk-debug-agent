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

#include "cAVS/Windows/MockedDevice.hpp"
#include "cAVS/Windows/MockedDeviceCatchHelper.hpp"
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

using Fixture = MockedDeviceFixture;

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
    // Ignore the "leftover input" callback since we want to manually test that there are, in fact,
    // leftover inputs.
    MockedDevice device([] {});
    device.addSuccessfulIoctlEntry(IoCtl2, nullptr, nullptr, nullptr);
    device.addSuccessfulIoctlEntry(IoCtl1, nullptr, nullptr, nullptr);

    CHECK_NOTHROW(device.ioControl(IoCtl2, nullptr, nullptr));
    CHECK_FALSE(device.consumed());
}
