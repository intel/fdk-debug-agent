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
#include "cAVS/Windows/MockedDeviceCommands.hpp"
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

static const std::vector<char> fwConfigTlvList {
    /* Tag for FW_VERSION: 0x00000000 */
    0x00, 0x00, 0x00, 0x00,
    /* Length = 8 bytes */
    0x08, 0x00, 0x00, 0x00,
    /* Value: dsp_fw::FwVersion */
        /* major and minor */
        0x01, 0x02, 0x03, 0x04,
        /* hot fix and build */
        0x05, 0x06, 0x07, 0x08
};
static const size_t fwVersionValueOffsetInTlv = 8;

/** Produce a module entry vector of the supplied size.
 * Each entry is filled with an arbitrary content. */
std::vector<ModuleEntry> produceModuleEntries(std::size_t expectedModuleCount)
{
    ModuleEntry moduleEntry;
    setArbitraryContent(moduleEntry);

    std::vector<ModuleEntry> entries;
    for (std::size_t i = 0; i < expectedModuleCount; ++i) {
        entries.push_back(moduleEntry);
    }

    return entries;
}

/** Perform a module entry ioctl and check the result using the supplied expected module count */
void checkModuleEntryIoctl(windows::ModuleHandler& moduleHandler, std::size_t expectedModuleCount)
{
    /*Successful get module info command */
    std::vector<ModuleEntry> entries;
    CHECK_NOTHROW(moduleHandler.getModulesEntries(entries));

    /* Checking result */
    ModuleEntry expectedModuleEntry;
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
    MockedDeviceCommands commands(device);

    /* Simulating an os error during getting fw config */
    commands.addGetFwConfigCommand(
        false,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        fwConfigTlvList); /* unused parameter */

    /* Simulating a driver error during getting fw config */
    commands.addGetFwConfigCommand(
        true,
        STATUS_FLOAT_DIVIDE_BY_ZERO,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        fwConfigTlvList); /* unused parameter */

    /* Simulating a firmware error during getting fw config */
    commands.addGetFwConfigCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_FAILURE,
        fwConfigTlvList); /* unused parameter */

    /* Successful get fw config command */
    commands.addGetFwConfigCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        fwConfigTlvList);

    /* Simulating an os error during getting module entries */
    commands.addGetModuleEntriesCommand(
        false,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        std::vector<ModuleEntry>()); /* unused parameter */

    /* Simulating a driver error during getting module entries */
    commands.addGetModuleEntriesCommand(
        true,
        STATUS_FLOAT_DIVIDE_BY_ZERO,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        std::vector<ModuleEntry>()); /* unused parameter */

    /* Simulating a firmware error during getting module entries */
    commands.addGetModuleEntriesCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_FAILURE,
        std::vector<ModuleEntry>()); /* unused parameter */

    /* Successful get module info command with 2 modules */
    commands.addGetModuleEntriesCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        produceModuleEntries(2));

    /* Successful get module info command with 'MaxModuleCount' modules */
    commands.addGetModuleEntriesCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        produceModuleEntries(dsp_fw::MaxModuleCount));

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error during getting fw config */
    FwConfig fwConfig;
    CHECK_THROWS_MSG(moduleHandler.getFwConfig(fwConfig),
        "Device returns an exception: OS says that io control has failed.");
    CHECK(fwConfig.isFwVersionValid == false);

    /* Simulating a driver error during getting fw config */
    CHECK_THROWS_MSG(moduleHandler.getFwConfig(fwConfig),
        "Driver returns invalid status: " +
        std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(fwConfig.isFwVersionValid == false);

    /* Simulating a firmware error during getting fw config */
    CHECK_THROWS_MSG(moduleHandler.getFwConfig(fwConfig),
        "Firmware returns invalid status: " +
        std::to_string(static_cast<uint32_t>(dsp_fw::Message::ADSP_IPC_FAILURE)));
    CHECK(fwConfig.isFwVersionValid == false);

    /* Successful get fw config command */
    CHECK_NOTHROW(moduleHandler.getFwConfig(fwConfig));
    CHECK(fwConfig.isFwVersionValid == true);
    const dsp_fw::FwVersion *injectedVersion =
        reinterpret_cast<const dsp_fw::FwVersion *>
            (fwConfigTlvList.data() + fwVersionValueOffsetInTlv);
    // No operator== in FW type: compare each field individually:
    CHECK(fwConfig.fwVersion.major == injectedVersion->major);
    CHECK(fwConfig.fwVersion.minor == injectedVersion->minor);
    CHECK(fwConfig.fwVersion.hotfix == injectedVersion->hotfix);
    CHECK(fwConfig.fwVersion.build == injectedVersion->build);

    /* Simulating an os error during getting module entries */
    std::vector<ModuleEntry> entries;
    CHECK_THROWS_MSG(moduleHandler.getModulesEntries(entries),
        "Device returns an exception: OS says that io control has failed.");
    CHECK(entries.empty());

    /* Simulating a driver error during getting module entries */
    CHECK_THROWS_MSG(moduleHandler.getModulesEntries(entries),
        "Driver returns invalid status: " +
        std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(entries.empty());

    /* Simulating a firmware error during getting module entries */
    CHECK_THROWS_MSG(moduleHandler.getModulesEntries(entries),
        "Firmware returns invalid status: " +
        std::to_string(static_cast<uint32_t>(dsp_fw::Message::ADSP_IPC_FAILURE)));

    CHECK(entries.empty());

    /*Successful get module info command with 2 modules*/
    checkModuleEntryIoctl(moduleHandler, 2);

    /*Successful get module info command with 'MaxModuleCount' modules*/
    checkModuleEntryIoctl(moduleHandler, dsp_fw::MaxModuleCount);
}

