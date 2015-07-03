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
#include "cAVS/Windows/Logger.hpp"
#include <catch.hpp>
#include <memory>

using namespace debug_agent::cavs;
using namespace debug_agent::cavs::windows;

/** Add a "set logger parameters" command into the mocked device test vector */
void addLoggerSetParameterCommand(MockedDevice &device)
{
    /* Expected buffer, used as both expected input AND output buffer */
    TinyCmdLogParameterIoctl expected;

    driver::FwLogsState &logState = expected.getFwLogsState();
    logState.started = driver::LOG_STATE::STARTED;
    logState.output = driver::LOG_OUTPUT::OUTPUT_SRAM;
    logState.level = driver::LOG_LEVEL::HIGH;

    /* Returned output buffer*/
    TinyCmdLogParameterIoctl returned(expected);

    /* Result codes */
    returned.getTinyCmd().Body.Status = STATUS_SUCCESS;

    /* Adding entry */
    device.addIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET, &expected.getBuffer(),
        &expected.getBuffer(), &returned.getBuffer(), true);
}

/** Add a "get logger parameters" command into the mocked device test vector */
void addLoggerGetParameterCommand(MockedDevice &device)
{
    /* Expected buffer, used as both expected input AND output buffer */
    TinyCmdLogParameterIoctl expected;

    /* Returned output buffer*/
    TinyCmdLogParameterIoctl returned(expected);

    driver::FwLogsState &logState = returned.getFwLogsState();
    logState.started = driver::LOG_STATE::STARTED;
    logState.output = driver::LOG_OUTPUT::OUTPUT_SRAM;
    logState.level = driver::LOG_LEVEL::HIGH;

    /* Result codes */
    returned.getTinyCmd().Body.Status = STATUS_SUCCESS;

    /* Adding entry */
    device.addIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET, &expected.getBuffer(),
        &expected.getBuffer(), &returned.getBuffer(), true);
}

TEST_CASE("Logging: setting and getting parameters")
{
    MockedDevice device;

    /* Setting the test vector
     * ----------------------- */

    addLoggerSetParameterCommand(device);
    addLoggerGetParameterCommand(device);

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the windows logger, that will use the mocked device*/
    windows::Logger logger(device);

    /* Defining parameters that will be used for set then get*/
    windows::Logger::Parameters inputParameters(true, windows::Logger::Level::High,
        windows::Logger::Output::Sram);

    /* Setting the parameters */
    CHECK_NOTHROW(logger.setParameters(inputParameters));

    /* Getting the parameters */
    windows::Logger::Parameters outputParameters;
    CHECK_NOTHROW(outputParameters = logger.getParameters());

    /* Checking that returned parameters are correct */
    CHECK(outputParameters == inputParameters);

    /* Checking that test vector is fully consumed */
    CHECK_NOTHROW(device.checkMockingSuccess());
}

