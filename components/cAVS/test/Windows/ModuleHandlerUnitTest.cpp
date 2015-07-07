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

/** Produce a module entry vector of the supplied size.
 * Each entry is filled with an arbitrary content. */
std::vector<dsp_fw::ModuleEntry> produceModuleEntries(std::size_t expectedModuleCount)
{
    dsp_fw::ModuleEntry moduleEntry;
    setArbitraryContent(moduleEntry);

    std::vector<dsp_fw::ModuleEntry> entries;
    for (std::size_t i = 0; i < expectedModuleCount; ++i) {
        entries.push_back(moduleEntry);
    }

    return entries;
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

    /* Simulating a driver error during getting adsp properties */
    MockedDeviceCommands commands(device);
    commands.addGetAdspPropertiesCommand(
        STATUS_FLOAT_DIVIDE_BY_ZERO,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        dsp_fw::AdspProperties()); /* unused parameter */

    /* Simulating a firmware error during getting adsp properties */
    commands.addGetAdspPropertiesCommand(
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_FAILURE,
        dsp_fw::AdspProperties()); /* unused parameter */

    /* Successful get adsp properties command */
    dsp_fw::AdspProperties expectedAdspProperties;
    setArbitraryContent(expectedAdspProperties);
    commands.addGetAdspPropertiesCommand(
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        expectedAdspProperties);

    /* Simulating a driver error during getting module entries */
    commands.addGetModuleEntriesCommand(
        STATUS_FLOAT_DIVIDE_BY_ZERO,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        std::vector<dsp_fw::ModuleEntry>()); /* unused parameter */

    /* Simulating a firmware error during getting module entries */
    commands.addGetModuleEntriesCommand(
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_FAILURE,
        std::vector<dsp_fw::ModuleEntry>()); /* unused parameter */

    /* Successful get module info command with 2 modules */
    commands.addGetModuleEntriesCommand(
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        produceModuleEntries(2));

    /* Successful get module info command with 'MaxModuleCount' modules */
    commands.addGetModuleEntriesCommand(
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        produceModuleEntries(dsp_fw::MaxModuleCount));

    /* Now using the mocked device
     * --------------------------- */

    dsp_fw::AdspProperties emptyAdspProperties;
    memset(&emptyAdspProperties, 0, sizeof(dsp_fw::AdspProperties));

    dsp_fw::AdspProperties properties = emptyAdspProperties;

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating a driver error during getting adsp properties */
    CHECK_THROWS_MSG(moduleHandler.getAdspProperties(properties),
        "Driver returns invalid status: " +
        std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(memoryEquals(properties, emptyAdspProperties));

    /* Simulating a firmware error during getting adsp properties */
    CHECK_THROWS_MSG(moduleHandler.getAdspProperties(properties),
        "Firmware returns invalid status: " +
        std::to_string(static_cast<uint32_t>(dsp_fw::Message::ADSP_IPC_FAILURE)));
    CHECK(memoryEquals(properties, emptyAdspProperties));

    /* Successful get adsp properties command */
    CHECK_NOTHROW(moduleHandler.getAdspProperties(properties));
    CHECK(memoryEquals(properties, expectedAdspProperties));


    /* Simulating a driver error during getting module entries */
    std::vector<dsp_fw::ModuleEntry> entries;
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

