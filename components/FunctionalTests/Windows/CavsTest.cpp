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

#include "Core/DebugAgent.hpp"
#include "Util/Uuid.hpp"
#include "TestCommon/HttpClientSimulator.hpp"
#include "cAVS/Windows/DeviceInjectionDriverFactory.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "cAVS/Windows/StubbedWppClientFactory.hpp"
#include "catch.hpp"

using namespace debug_agent::core;
using namespace debug_agent::cavs;
using namespace debug_agent::test_common;
using namespace debug_agent::util;

static const std::size_t ModuleUIDSize = 4;
using ModuleUID = uint32_t[ModuleUIDSize];

/** Defining some module uuids... */
const Uuid Module0UID = { 0x01020304, 0x0506, 0x0708,
    { 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10}};

const Uuid Module1UID = { 0x11121314, 0x1516, 0x1718,
    { 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20 } };

/** Helper function to set a module entry */
void setModuleEntry(dsp_fw::ModuleEntry &entry, const std::string &name,
    const Uuid &uuid)
{
    /* Setting name */
    assert(name.size() <= sizeof(entry.name));
    for (std::size_t i = 0; i < sizeof(entry.name); i++) {
        if (i < name.size()) {
            entry.name[i] = name[i];
        }
        else {
            /* Filling buffer with 0 after name end */
            entry.name[i] = 0;
        }
    }

    /* Setting GUID*/
    uuid.toOtherUuidType(entry.uuid);
}

void addModuleEntryCommand(windows::MockedDeviceCommands &commands)
{
    /* Creating 2 module entries */
    std::vector<dsp_fw::ModuleEntry> returnedEntries(2);
    setModuleEntry(returnedEntries[0], "module_0", Module0UID);
    setModuleEntry(returnedEntries[1], "module_1", Module1UID);

    /* Adding the "get module entries" command to the test vector */
    commands.addGetModuleEntriesCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        returnedEntries);
}

TEST_CASE("DebugAgent/cAVS: module entries")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
     * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial module entry command */
    addModuleEntryCommand(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* The expected content is an html table describing the mapping uuid<-> module_id */
    std::string expectedContent(
        "<p>Module type count: 2</p>"
        "<table border='1'><tr><td>name</td><td>uuid</td><td>module id</td></tr>"
        "<tr><td>module_0</td><td>01020304-0506-0708-090A-0B0C0D0E0F10</td><td>0</td></tr>"
        "<tr><td>module_1</td><td>11121314-1516-1718-191A-1B1C1D1E1F20</td><td>1</td></tr>"
        "</table>");

    /* Doing the request on the "/cAVS/module/entries" URI */
    CHECK_NOTHROW(client.request(
        "/cAVS/module/entries",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/html",
        expectedContent
        ));
}

TEST_CASE("DebugAgent/cAVS: log parameters")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* 0: Initial module entry command */
    addModuleEntryCommand(commands);

    /* 1: Get log parameter, will return
     * - isStarted : false
     * - level: critical
     * - output: pti
     */

    windows::driver::FwLogsState initialLogParams = {
        false,
        windows::driver::LOG_LEVEL::CRITICAL,
        windows::driver::LOG_OUTPUT::OUTPUT_PTI
    };
    commands.addGetLogParametersCommand(
        true,
        STATUS_SUCCESS,
        initialLogParams);

    /* 2: Set log parameter to
    * - isStarted : true
    * - level: verbose
    * - output: sram
    */
    windows::driver::FwLogsState setLogParams = {
        true,
        windows::driver::LOG_LEVEL::VERBOSE,
        windows::driver::LOG_OUTPUT::OUTPUT_SRAM
    };
    commands.addSetLogParametersCommand(
        true,
        STATUS_SUCCESS,
        setLogParams);

    /* 3: Get log parameter , will return
    * - isStarted : true
    * - level: verbose
    * - output: sram
    */
    commands.addGetLogParametersCommand(
        true,
        STATUS_SUCCESS,
        setLogParams);

    /** Adding a successful set log parameters command, this is called by the System class
     * destructor to stop log */
    setLogParams = {
        windows::driver::LOG_STATE::STOPPED,
        windows::driver::LOG_LEVEL::VERBOSE,
        windows::driver::LOG_OUTPUT::OUTPUT_SRAM
    };
    commands.addSetLogParametersCommand(
        true,
        STATUS_SUCCESS,
        setLogParams);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting log parameters*/
    CHECK_NOTHROW(client.request(
        "/cAVS/logging/parameters",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/html",
        "<table border='1'>"
        "<tr><th>Log Parameter</th><th>Value</th></tr>"
        "<tr><td>State</td><td>0</td></tr>"
        "<tr><td>Level</td><td>Critical</td></tr>"
        "<tr><td>Output</td><td>PTI</td></tr>"
        "</table><p>To change log parameters: PUT [log status];[log level];[log output] at "
        "/cAVS/logging/parameters</p>"
        ));

    /* 2: Setting log parameters */
    CHECK_NOTHROW(client.request(
        "/cAVS/logging/parameters",
        HttpClientSimulator::Verb::Put,
        "1;Verbose;SRAM",
        HttpClientSimulator::Status::Ok,
        "text/html",
        "<p>Done</p>"
        ));

    /* 3: Getting log parameters again */
    CHECK_NOTHROW(client.request(
        "/cAVS/logging/parameters",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/html",
        "<table border='1'>"
        "<tr><th>Log Parameter</th><th>Value</th></tr>"
        "<tr><td>State</td><td>1</td></tr>"
        "<tr><td>Level</td><td>Verbose</td></tr>"
        "<tr><td>Output</td><td>SRAM</td></tr>"
        "</table><p>To change log parameters: PUT [log status];[log level];[log output] at "
        "/cAVS/logging/parameters</p>"
        ));
}