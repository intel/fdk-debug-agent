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
#include "cAVS/Windows/ModuleHandler.hpp"
#include <catch.hpp>
#include <memory>
#include <iostream>

/**
 * NOTE: test vector buffers are filled with firmware and driver types, maybe a better way
 * would consist in using a bitstream in order to construct ioctl buffers from scratch.
 * To be discussed...
 */

using namespace debug_agent::cavs;
using namespace debug_agent::cavs::windows;

/** Compares type memory content */
template <typename T>
bool memoryEquals(const T &v1, const T &v2)
{
    return memcmp(&v1, &v2, sizeof(T)) == 0;
}

/** Set the memory of one type with arbitrary content */
template <typename T>
void setArbitraryContent(T &value)
{
    uint8_t *buf = reinterpret_cast<uint8_t*>(&value);
    for (std::size_t i = 0; i < sizeof(T); i++) {
        buf[i] = static_cast<uint8_t>(i);
    }
}

/** Add a get Adsp properties command into the mocked device test vector */
void addAdspPropertiesCommand(MockedDevice &device, NTSTATUS driverStatus,
    dsp_fw::Message::IxcStatus firmwareStatus)
{
    /* Expected output buffer*/
    BigCmdModuleAccessIoctlOutput<dsp_fw::AdspProperties> expectedOutput(
        dsp_fw::BaseFwParams::ADSP_PROPERTIES, sizeof(dsp_fw::AdspProperties));

    /* Returned output buffer*/
    BigCmdModuleAccessIoctlOutput<dsp_fw::AdspProperties> returnedOutput(
        dsp_fw::BaseFwParams::ADSP_PROPERTIES, sizeof(dsp_fw::AdspProperties));

    /* Result codes */
    returnedOutput.getCmdBody().Status = driverStatus;
    returnedOutput.getModuleParameterAccess().FwStatus = firmwareStatus;

    /* Adsp properties content */
    setArbitraryContent(returnedOutput.getFirmwareParameter());

    /* Filling expected input buffer */
    TypedBuffer<driver::Intc_App_Cmd_Header> expectedInput;
    expectedInput->FeatureID =
        static_cast<ULONG>(driver::FEATURE_MODULE_PARAMETER_ACCESS);
    expectedInput->ParameterID = 0; /* only one parameter id for this feature */
    expectedInput->DataSize = static_cast<ULONG>(expectedOutput.getBuffer().getSize());

    /* Adding entry */
    device.addIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET, &expectedInput,
        &expectedOutput.getBuffer(), &returnedOutput.getBuffer(), true);
}

/** Add a get module info command into the mocked device test vector */
void addModuleInfoCommand(MockedDevice &device, std::size_t moduleCount)
{
    std::size_t moduleInfoSize = ModulesInfoHelper::getAllocationSize();

    /* Expected output buffer*/
    BigCmdModuleAccessIoctlOutput<dsp_fw::ModulesInfo>
        expectedOutput(dsp_fw::MODULES_INFO_GET, moduleInfoSize);

    /* Returned output buffer*/
    BigCmdModuleAccessIoctlOutput<dsp_fw::ModulesInfo>
        returnedOutput(dsp_fw::MODULES_INFO_GET, moduleInfoSize);

    /* Result codes */
    returnedOutput.getCmdBody().Status = STATUS_SUCCESS;
    returnedOutput.getModuleParameterAccess().FwStatus =
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS;

    /* Settings module entry content*/
    dsp_fw::ModulesInfo &modulesInfo = returnedOutput.getFirmwareParameter();
    modulesInfo.module_count = static_cast<uint32_t>(moduleCount);
    for (std::size_t i = 0; i < moduleCount; i++) {
        setArbitraryContent(modulesInfo.module_info[i]);
    }

    /* Filling expected input buffer */
    TypedBuffer<driver::Intc_App_Cmd_Header> expectedInput;
    expectedInput->FeatureID = static_cast<ULONG>(driver::FEATURE_MODULE_PARAMETER_ACCESS);
    expectedInput->ParameterID = 0; /* only one parameter id for this feature */
    expectedInput->DataSize = static_cast<ULONG>(expectedOutput.getBuffer().getSize());

    /* Adding entry */
    device.addIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET, &expectedInput,
        &expectedOutput.getBuffer(), &returnedOutput.getBuffer(), true);
}

/** Perform a module entry ioctl and check the result using the supplied expected module count */
void checkModuleEntryIoctl(windows::ModuleHandler& moduleHandler, std::size_t expectedModuleCount)
{
    /*Successful get module info command */
    std::vector<dsp_fw::ModuleEntry> entries;
    CHECK_NOTHROW(moduleHandler.getModulesEntries(entries));

    /* Checking result */
    dsp_fw::ModuleEntry expectedModuleEntry;
    setArbitraryContent(expectedModuleEntry);
    CHECK(entries.size() == expectedModuleCount);
    for (auto &candidateModuleEntry : entries) {
        CHECK(memoryEquals(candidateModuleEntry, expectedModuleEntry));
    }
}

TEST_CASE("Module handling: getting module entries")
{
    MockedDevice device;

    /* Setting the test vector
     * ----------------------- */

    /* Simulating a driver error */
    addAdspPropertiesCommand(device, STATUS_FLOAT_DIVIDE_BY_ZERO,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS);

    /* Simulating a firmware error */
    addAdspPropertiesCommand(device, STATUS_SUCCESS, dsp_fw::Message::IxcStatus::ADSP_IPC_FAILURE);

    /* Successful get adsp properties command */
    addAdspPropertiesCommand(device, STATUS_SUCCESS, dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS);

    /* Successful get module info command with 2 modules */
    addModuleInfoCommand(device, 2);

    /* Successful get module info command with 'MaxModuleCount' modules */
    addModuleInfoCommand(device, dsp_fw::MaxModuleCount);

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    dsp_fw::AdspProperties properties;

    /* Simulating a driver error */
    CHECK_THROWS_MSG(moduleHandler.getAdspProperties(properties),
        "Driver returns invalid status: 3221225614"); /* value of STATUS_FLOAT_DIVIDE_BY_ZERO */

    /* Simulating a firmware error */
    CHECK_THROWS_MSG(moduleHandler.getAdspProperties(properties),
        "Firmware returns invalid status: 6"); /* value of ADSP_IPC_FAILURE */

    /* Successful get adsp properties command */
    CHECK_NOTHROW(moduleHandler.getAdspProperties(properties));

    /* Checking result */
    dsp_fw::AdspProperties expectedProperties;
    setArbitraryContent(expectedProperties);
    CHECK(memoryEquals(properties, expectedProperties));

    /*Successful get module info command with 2 modules*/
    checkModuleEntryIoctl(moduleHandler, 2);

    /*Successful get module info command with 'MaxModuleCount' modules*/
    checkModuleEntryIoctl(moduleHandler, dsp_fw::MaxModuleCount);

    /* Checking that test vector is fully consumed */
    CHECK_NOTHROW(device.checkMockingSuccess());
}

