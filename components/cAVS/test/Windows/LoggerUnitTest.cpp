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

#include "TestCommon/TestHelpers.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include "cAVS/Windows/MockedDeviceCatchHelper.hpp"
#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "cAVS/Windows/StubbedWppClientFactory.hpp"
#include "cAVS/Windows/Logger.hpp"
#include <catch.hpp>
#include <memory>

using namespace debug_agent::cavs;
using namespace debug_agent::cavs::windows;

using Fixture = MockedDeviceFixture;

TEST_CASE_METHOD(Fixture, "Logging: setting and getting parameters")
{
    StubbedWppClientFactory wppClientFactory;

    /* Setting the test vector
     * ----------------------- */

    MockedDeviceCommands commands(device);

    driver::IoctlFwLogsState expectedFwLogState = {driver::IOCTL_LOG_STATE::STARTED,
                                                   driver::FW_LOG_LEVEL::LOG_HIGH,
                                                   driver::FW_LOG_OUTPUT::OUTPUT_WPP};

    /** Adding a failed set log parameters command due to OS error */
    commands.addSetLogParametersCommand(false, STATUS_SUCCESS, expectedFwLogState);

    /** Adding a failed set log parameters command due to driver error */
    commands.addSetLogParametersCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO, expectedFwLogState);

    /** Adding a successful set log parameters command */
    commands.addSetLogParametersCommand(true, STATUS_SUCCESS, expectedFwLogState);

    /** Adding a failed get log parameters command due to OS error */
    commands.addGetLogParametersCommand(false, STATUS_SUCCESS,
                                        driver::IoctlFwLogsState()); /* Unused parameter */

    /** Adding a failed get log parameters command due to driver error */
    commands.addGetLogParametersCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                        driver::IoctlFwLogsState()); /* Unused parameter */

    /** Adding a successful get log parameters command */
    commands.addGetLogParametersCommand(true, STATUS_SUCCESS, expectedFwLogState);

    /** Adding a successful set log parameters command, this is called by the logger destructor
     * to stop log */
    expectedFwLogState = {driver::IOCTL_LOG_STATE::STOPPED, driver::FW_LOG_LEVEL::LOG_VERBOSE,
                          driver::FW_LOG_OUTPUT::OUTPUT_WPP};
    commands.addSetLogParametersCommand(true, STATUS_SUCCESS, expectedFwLogState);

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the windows logger, that will use the mocked device*/
    windows::Logger logger(device, wppClientFactory);

    /* Defining parameters that will be used for set then get*/
    windows::Logger::Parameters inputParameters(true, windows::Logger::Level::High,
                                                windows::Logger::Output::Sram);

    /* Checking that set log parameters command produces OS error */
    CHECK_THROWS_AS_MSG(logger.setParameters(inputParameters), windows::Logger::Exception,
                        "TinySet error: OS says that io control has failed.");

    /* Checking that set log parameters command produces driver error */
    CHECK_THROWS_AS_MSG(logger.setParameters(inputParameters), windows::Logger::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));

    /* Checking successful set log parameters command */
    CHECK_NOTHROW(logger.setParameters(inputParameters));

    /* Checking that get log parameters command produces OS error */
    CHECK_THROWS_AS_MSG(logger.getParameters(), windows::Logger::Exception,
                        "TinyGet error: OS says that io control has failed.");

    /* Checking that get log parameters command produces driver error */
    CHECK_THROWS_AS_MSG(logger.getParameters(), windows::Logger::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));

    /* Checking successful get log parameters command */
    windows::Logger::Parameters outputParameters;
    CHECK_NOTHROW(outputParameters = logger.getParameters());

    /* Checking that returned parameters are correct */
    CHECK(outputParameters == inputParameters);
}
