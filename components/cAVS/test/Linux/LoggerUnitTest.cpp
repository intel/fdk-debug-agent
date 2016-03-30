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

#include "TestCommon/TestHelpers.hpp"
#include "cAVS/Linux/StubbedControlDevice.hpp"
#include "cAVS/Linux/MockedControlDeviceCommands.hpp"
#include "cAVS/Linux/MockedDeviceCommands.hpp"
#include "cAVS/Linux/MockedDeviceCatchHelper.hpp"
#include "cAVS/Linux/StubbedCompressDeviceFactory.hpp"
#include "cAVS/Linux/MockedCompressDeviceFactory.hpp"
#include "cAVS/Linux/Logger.hpp"
#include <catch.hpp>

using namespace debug_agent::cavs;
using namespace debug_agent::util;
using namespace debug_agent::cavs::linux;

using Fixture = MockedDeviceFixture;

TEST_CASE_METHOD(Fixture, "Logging: setting and getting parameters with Stub")
{
    StubbedCompressDeviceFactory compressDeviceFactory;

    /* Setting the test vector
     * ----------------------- */
    MockedDeviceCommands commands(*device);

    commands.addSetCorePowerCommand(true, 0, false);
    commands.addSetCorePowerCommand(true, 1, false);
    commands.addSetCorePowerCommand(true, 0, true);
    commands.addSetCorePowerCommand(true, 1, true);

    /* Now using the stubbed compress device
     * -------------------------------------- */

    /* Creating the windows logger, that will use the mocked device*/
    linux::Logger logger(*device, *controlDevice, compressDeviceFactory);

    /* Defining parameters that will be used for set then get*/
    linux::Logger::Parameters inputParameters(true, debug_agent::cavs::Logger::Level::Verbose,
                                              debug_agent::cavs::Logger::Output::Sram);

    /* Checking successful set log parameters command */
    CHECK_NOTHROW(logger.setParameters(inputParameters));

    /* Checking that set log parameters command is supported if running*/
    CHECK_NOTHROW(logger.setParameters(inputParameters));

    /* Checking successful get log parameters command */
    debug_agent::cavs::Logger::Parameters outputParameters;
    CHECK_NOTHROW(outputParameters = logger.getParameters());

    /* Checking that returned parameters are correct */
    CHECK(outputParameters == inputParameters);
}

TEST_CASE_METHOD(Fixture, "Logging: setting and getting parameters with Open Fail Mock")
{
    /* Setting the test vector
     * ----------------------- */
    MockedDeviceCommands commands(*device);

    commands.addSetCorePowerCommand(true, 0, false);
    commands.addSetCorePowerCommand(true, 0, true);

    MockedCompressDeviceFactory compressDeviceFactory;
    compressDevice->addFailedCompressDeviceEntryOpen();
    compressDeviceFactory.addMockedDevice(std::move(compressDevice));

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the windows logger, that will use the mocked device*/
    linux::Logger logger(*device, *controlDevice, compressDeviceFactory);

    /* Defining parameters that will be used for set then get*/
    linux::Logger::Parameters inputParameters(true, debug_agent::cavs::Logger::Level::Verbose,
                                              debug_agent::cavs::Logger::Output::Sram);

    /* Checking that set log parameters command produces OS error if cannot open log device*/
    CHECK_THROWS_AS_MSG(logger.setParameters(inputParameters), linux::Logger::Exception,
                        "Error opening Log Device: error during compress open: error#MockDevice");
}

TEST_CASE_METHOD(Fixture, "Logging: setting and getting parameters with Start Fail Mock")
{
    /* Setting the test vector
     * ----------------------- */
    MockedDeviceCommands commands(*device);

    commands.addSetCorePowerCommand(true, 0, false);
    commands.addSetCorePowerCommand(true, 0, true);

    MockedCompressDeviceFactory compressDeviceFactory;

    compressDevice->addSuccessfulCompressDeviceEntryOpen();
    compressDevice->addFailedCompressDeviceEntryStart();
    compressDeviceFactory.addMockedDevice(std::move(compressDevice));

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the windows logger, that will use the mocked device*/
    linux::Logger logger(*device, *controlDevice, compressDeviceFactory);

    /* Defining parameters that will be used for set then get*/
    linux::Logger::Parameters inputParameters(true, debug_agent::cavs::Logger::Level::Verbose,
                                              debug_agent::cavs::Logger::Output::Sram);

    /* Checking that set log parameters command produces OS error if cannot start log device*/
    CHECK_THROWS_AS_MSG(logger.setParameters(inputParameters), linux::Logger::Exception,
                        "Error starting Log Device: error during compress start: error#MockDevice");
}

TEST_CASE_METHOD(Fixture, "Logging: setting and getting parameters with Stop Fail Mock")
{
    /* Setting the test vector
     * ----------------------- */
    MockedDeviceCommands commands(*device);

    commands.addSetCorePowerCommand(true, 0, false);
    commands.addSetCorePowerCommand(true, 0, true);

    MockedCompressDeviceFactory compressDeviceFactory;
    compressDevice->addSuccessfulCompressDeviceEntryOpen();
    compressDevice->addSuccessfulCompressDeviceEntryStart();
    compressDevice->addFailedCompressDeviceEntryStop();
    compressDeviceFactory.addMockedDevice(std::move(compressDevice));

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the windows logger, that will use the mocked device*/
    linux::Logger logger(*device, *controlDevice, compressDeviceFactory);

    /* Defining parameters that will be used for set then get*/
    linux::Logger::Parameters inputParameters(true, debug_agent::cavs::Logger::Level::Verbose,
                                              debug_agent::cavs::Logger::Output::Sram);

    /* Checking successful set log parameters command */
    CHECK_NOTHROW(logger.setParameters(inputParameters));

    /* Now stopping the log, */
    inputParameters = linux::Logger::Parameters(false, debug_agent::cavs::Logger::Level::Verbose,
                                                debug_agent::cavs::Logger::Output::Sram);

    /* Checking that set log parameters command produces OS error if cannot close*/
    CHECK_NOTHROW(logger.setParameters(inputParameters));
}

TEST_CASE_METHOD(Fixture, "Logging: setting and getting parameters with Empty Logger device list")
{
    /* Setting the test vector
     * ----------------------- */

    MockedCompressDeviceFactory compressDeviceFactory;

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the windows logger, that will use the mocked device*/
    linux::Logger logger(*device, *controlDevice, compressDeviceFactory);

    /* Defining parameters that will be used for set then get*/
    linux::Logger::Parameters inputParameters(true, debug_agent::cavs::Logger::Level::Verbose,
                                              debug_agent::cavs::Logger::Output::Sram);

    /* Checking that set log parameters command produces OS error if cannot start*/
    CHECK_THROWS_AS_MSG(logger.setParameters(inputParameters), linux::Logger::Exception,
                        "No Log Producers instantiated.");
}
