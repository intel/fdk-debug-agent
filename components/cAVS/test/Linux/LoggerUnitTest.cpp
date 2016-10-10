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

TEST_CASE_METHOD(Fixture, "Logging: setting and getting parameters")
{
    /* Setting the test vector
     * ----------------------- */
    MockedDeviceCommands commands(*device);

    commands.addSetCorePowerCommand(true, 0, false);

    MockedControlDeviceCommands controlCommands(*controlDevice);

    /** Adding a failed set log parameters command due to control error. */
    controlCommands.addSetLogLevelCommand(false, mixer_ctl::LogPriority::Verbose);

    /** Adding a successful set log parameters command */
    controlCommands.addSetLogLevelCommand(true, mixer_ctl::LogPriority::Verbose);

    /** Adding a failed get log parameters command due to control error. */
    controlCommands.addGetLogLevelCommand(false, mixer_ctl::LogPriority::Verbose);

    /** Adding a successful get log parameters command */
    controlCommands.addGetLogLevelCommand(true, mixer_ctl::LogPriority::Verbose);

    commands.addSetCorePowerCommand(true, 0, true);

    MockedCompressDeviceFactory compressDeviceFactory;
    compressDevice->addSuccessfulCompressDeviceEntryOpen();
    compressDevice->addSuccessfulCompressDeviceEntryStart();

    compressDevice->addSuccessfulCompressDeviceEntryWait(CompressDevice::mInfiniteTimeout, false);

    compressDevice->addSuccessfulCompressDeviceEntryStop();
    compressDeviceFactory.addMockedDevice(std::move(compressDevice));

    /* Now using the mocked device
     * --------------------------- */
    //* Creating the windows logger, that will use the mocked device*/
    linux::Logger logger(*device, *controlDevice, compressDeviceFactory);

    /* Defining parameters that will be used for set then get*/
    linux::Logger::Parameters inputParameters(true, debug_agent::cavs::Logger::Level::Verbose,
                                              debug_agent::cavs::Logger::Output::Sram);

    /* Checking that set log parameters command produces OS error */
    CHECK_THROWS_AS_MSG(logger.setParameters(inputParameters), linux::Logger::Exception,
                        "Failed to write the log level control: Control Device says that control "
                        "Write has failed.");

    /* Checking successful set log parameters command */
    CHECK_NOTHROW(logger.setParameters(inputParameters));

    /* Checking that get log parameters command produces OS error */
    CHECK_THROWS_AS_MSG(
        logger.getParameters(), linux::Logger::Exception,
        "Failed to read the log level control: Control Device says that control Read has failed.");

    /* Checking successful get log parameters command */
    linux::Logger::Parameters outputParameters;
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

    MockedControlDeviceCommands controlCommands(*controlDevice);
    controlCommands.addSetLogLevelCommand(true, mixer_ctl::LogPriority::Verbose);

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

    MockedControlDeviceCommands controlCommands(*controlDevice);
    controlCommands.addSetLogLevelCommand(true, mixer_ctl::LogPriority::Verbose);

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

TEST_CASE_METHOD(Fixture, "Logging: setting and getting parameters with Empty Logger device list")
{
    /* Setting the test vector
     * ----------------------- */

    MockedCompressDeviceFactory compressDeviceFactory;

    MockedControlDeviceCommands controlCommands(*controlDevice);
    controlCommands.addSetLogLevelCommand(true, mixer_ctl::LogPriority::Verbose);

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
