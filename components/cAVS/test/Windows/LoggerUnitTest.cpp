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
                                                   driver::FW_LOG_OUTPUT::OUTPUT_SRAM};

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
                          driver::FW_LOG_OUTPUT::OUTPUT_SRAM};
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
