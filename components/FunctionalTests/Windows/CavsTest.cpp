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
#include "TestCommon/TestHelpers.hpp"
#include "cAVS/Windows/DeviceInjectionDriverFactory.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "cAVS/Windows/StubbedWppClientFactory.hpp"
#include "catch.hpp"
#include <chrono>
#include <thread>
#include <future>
#include <condition_variable>

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

static const std::vector<char> fwConfigTlvList {
    /* Tag for FW_VERSION: 0x00000000 */
    0x00, 0x00, 0x00, 0x00,
    /* Length = 8 bytes */
    0x08, 0x00, 0x00, 0x00,
    /* Value */
        /* major and minor */
        0x01, 0x02, 0x03, 0x04,
        /* hot fix and build */
        0x05, 0x06, 0x07, 0x08,
    /* Tag for MODULES_COUNT : 12 */
    12, 0x00, 0x00, 0x00,
    /* Length = 4 bytes */
    0x04, 0x00, 0x00, 0x00,
        /* Value : 2 */
        0x02, 0x00, 0x00, 0x00
};
static const size_t fwVersionValueOffsetInTlvList = 8;

static const std::vector<char> hwConfigTlvList {
    /* Tag for DSP_CORES: 0x00000001 */
    0x01, 0x00, 0x00, 0x00,
    /* Length = 4 bytes */
    0x04, 0x00, 0x00, 0x00,
    /* Value: nb core */
    0x01, 0x00, 0x00, 0x00
};
static const size_t nbCoreValueOffsetInTlvList = 8;

/** Helper function to set a module entry */
void setModuleEntry(ModuleEntry &entry, const std::string &name,
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
    std::vector<ModuleEntry> returnedEntries(2);
    setModuleEntry(returnedEntries[0], "module_0", Module0UID);
    setModuleEntry(returnedEntries[1], "module_1", Module1UID);

    /* Adding the "get module entries" command to the test vector */
    commands.addGetModuleEntriesCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        2,
        returnedEntries);
}

void addFwConfigCommand(windows::MockedDeviceCommands &commands)
{
    /* Adding the "get FW Config" command to the test vector */
    commands.addGetFwConfigCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        fwConfigTlvList);
}

void addHwConfigCommand(windows::MockedDeviceCommands &commands)
{
    /* Adding the "get HW Config" command to the test vector */
    commands.addGetHwConfigCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        hwConfigTlvList);
}

void addInitialCommand(windows::MockedDeviceCommands &commands)
{
    addFwConfigCommand(commands);
    addHwConfigCommand(commands);
    addModuleEntryCommand(commands);
}

TEST_CASE("DebugAgent/cAVS: module entries")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
     * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

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

TEST_CASE("DebugAgent/cAVS: system type (URL: /type)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/type",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<system_type Name=\"bxtn\">\n"
        "    <description>Broxton platform</description>\n"
        "    <characteristics/>\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <children>\n"
        "        <subsystem_collection Name=\"subsystems\">\n"
        "            <subsystem_type Name=\"cavs\"/>\n"
        "        </subsystem_collection>\n"
        "    </children>\n"
        "    <inputs/>\n"
        "    <outputs/>\n"
        "</system_type>\n"
        ));
}

TEST_CASE("DebugAgent/cAVS: system instance (URL: /instance)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/instance",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<system Id=\"0\" Type=\"bxtn\">\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <parents/>\n"
        "    <children>\n"
        "        <subsystem_collection Name=\"subsystems\">\n"
        "            <subsystem Id=\"0\" Type=\"cavs\"/>\n"
        "        </subsystem_collection>\n"
        "    </children>\n"
        "    <inputs/>\n"
        "    <outputs/>\n"
        "    <links/>\n"
        "</system>\n"
        ));
}

TEST_CASE("DebugAgent/cAVS: subsystem type (URL: /type/cavs)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    const dsp_fw::FwVersion *fwVersion = reinterpret_cast<const dsp_fw::FwVersion *>(
        fwConfigTlvList.data() + fwVersionValueOffsetInTlvList);
    const uint32_t *nbCores = reinterpret_cast<const uint32_t *>(
        hwConfigTlvList.data() + nbCoreValueOffsetInTlvList);

    /* 1: Getting subsystem information*/
    CHECK_NOTHROW(client.request(
        "/type/cavs",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<subsystem_type Name=\"cavs\">\n"
        "    <description>cAVS subsystem</description>\n"
        "    <characteristics>\n"
        "        <characteristic Name=\"Firmware version\">"
        + std::to_string(fwVersion->major) + "."
        + std::to_string(fwVersion->minor) + "."
        + std::to_string(fwVersion->hotfix) + "."
        + std::to_string(fwVersion->build)
        + "</characteristic>\n" +
        "        <characteristic Name=\"Current total number of module entries loaded "
        + "into the DSP\">2</characteristic>\n"
        "        <characteristic Name=\"Number of cores\">"
        + std::to_string(*nbCores)
        + "</characteristic>\n"
        "    </characteristics>\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <children>\n"
        "        <collection Name=\"pipes\">\n"
        "            <type Name=\"pipe\"/>\n"
        "        </collection>\n"
        "        <collection Name=\"cores\">\n"
        "            <type Name=\"core\"/>\n"
        "        </collection>\n"
        "        <collection Name=\"tasks\">\n"
        "            <type Name=\"task\"/>\n"
        "        </collection>\n"
        "        <service_collection Name=\"services\">\n"
        "            <service_type Name=\"fwlogs\"/>\n"
        "        </service_collection>\n"
        "        <component_collection Name=\"gateways\">\n"
        "            <component_type Name=\"hda-host-out-gateway\"/>\n"
        "            <component_type Name=\"hda-host-in-gateway\"/>\n"
        "            <component_type Name=\"hda-host-inout-gateway\"/>\n"
        "            <component_type Name=\"hda-link-out-gateway\"/>\n"
        "            <component_type Name=\"hda-link-in-gateway\"/>\n"
        "            <component_type Name=\"hda-link-inout-gateway\"/>\n"
        "            <component_type Name=\"dmic-link-in-gateway\"/>\n"
        "            <component_type Name=\"i2s-link-out-gateway\"/>\n"
        "            <component_type Name=\"i2s-link-in-gateway\"/>\n"
        "            <component_type Name=\"slimbus-link-out-gateway\"/>\n"
        "            <component_type Name=\"slimbus-link-in-gateway\"/>\n"
        "            <component_type Name=\"alh-link-out-gateway\"/>\n"
        "            <component_type Name=\"alh-link-in-gateway\"/>\n"
        "        </component_collection>\n"
        "        <component_collection Name=\"modules\">\n"
        "            <component_type Name=\"module_0\"/>\n"
        "            <component_type Name=\"module_1\"/>\n"
        "        </component_collection>\n"
        "    </children>\n"
        "    <inputs/>\n"
        "    <outputs/>\n"
        "    <categories>\n"
        "        <type Name=\"pipe\"/>\n"
        "        <type Name=\"core\"/>\n"
        "        <type Name=\"task\"/>\n"
        "        <service_type Name=\"fwlogs\"/>\n"
        "        <component_type Name=\"hda-host-out-gateway\"/>\n"
        "        <component_type Name=\"hda-host-in-gateway\"/>\n"
        "        <component_type Name=\"hda-host-inout-gateway\"/>\n"
        "        <component_type Name=\"hda-link-out-gateway\"/>\n"
        "        <component_type Name=\"hda-link-in-gateway\"/>\n"
        "        <component_type Name=\"hda-link-inout-gateway\"/>\n"
        "        <component_type Name=\"dmic-link-in-gateway\"/>\n"
        "        <component_type Name=\"i2s-link-out-gateway\"/>\n"
        "        <component_type Name=\"i2s-link-in-gateway\"/>\n"
        "        <component_type Name=\"slimbus-link-out-gateway\"/>\n"
        "        <component_type Name=\"slimbus-link-in-gateway\"/>\n"
        "        <component_type Name=\"alh-link-out-gateway\"/>\n"
        "        <component_type Name=\"alh-link-in-gateway\"/>\n"
        "        <component_type Name=\"module_0\"/>\n"
        "        <component_type Name=\"module_1\"/>\n"
        "    </categories>\n"
        "</subsystem_type>\n"
        ));
}

TEST_CASE("DebugAgent/cAVS: subsystem instances (URL: /instance/cavs)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<subsystem_collection>"
        "    <subsystem Type=\"cavs\" Id=\"0\">"
        "        <info_parameters>"
        "            <ParameterBlock Name=\"Free Pages\">"
        "                <ParameterBlock Name=\"0\">"
        "                    <EnumParameter Name=\"mem_type\">HP_MEM</EnumParameter>"
        "                    <IntegerParameter Name=\"pages\">12</IntegerParameter>"
        "                </ParameterBlock>"
        "                <ParameterBlock Name=\"1\">"
        "                    <EnumParameter Name=\"mem_type\">LP_MEM</EnumParameter>"
        "                    <IntegerParameter Name=\"pages\">13</IntegerParameter>"
        "                </ParameterBlock>"
        "            </ParameterBlock>"
        "        </info_parameters>"
        "        <parents>"
        "            <system Type=\"SKL\" Id=\"0\"/>"
        "        </parents>"
        "        <children>"
        "            <collection Name=\"pipes\">"
        "                <!-- all pipe instances -->"
        "                <instance Type=\"pipe\" Id=\"0\"/>"
        "                <instance Type=\"pipe\" Id=\"1\"/>"
        "            </collection>"
        "            <collection Name=\"cores\">"
        "                <!-- all core instances -->"
        "                <instance Type=\"core\" Id=\"0\"/>"
        "                <instance Type=\"core\" Id=\"1\"/>"
        "            </collection>"
        "            <service_collection Name=\"services\">"
        "                <service Type=\"fwlogs\" Id=\"0\"/>"
        "            </service_collection>"
        "            <component_collection Name=\"modules\">"
        "                <!-- all module instances -->"
        "                <component Type=\"module-aec(2)\" Id=\"0\"/>"
        "                <component Type=\"module-gain(4)\" Id=\"3\"/>"
        "                <component Type=\"module-copier(1)\" Id=\"2\"/>"
        "            </component_collection>"
        "        </children>"
        "        <!-- links -->"
        "        <links>"
        "            <link Id=\"0\">"
        "                <from Type=\"module-aec(2)\" Id=\"0\" OutputId=\"1\"/>"
        "                <to Type=\"module-gain(4)\" Id=\"3\" InputId=\"0\"/>"
        "            </link>"
        "            <link Id=\"1\">"
        "                <from Type=\"module-gain(4)\" Id=\"3\" OutputId=\"2\"/>"
        "                <to Type=\"module-copier(1)\" Id=\"2\" InputId=\"0\"/>"
        "            </link>"
        "        </links>"
        "    </subsystem>"
        "</subsystem_collection>"
        ));
}

TEST_CASE("DebugAgent/cAVS: subsystem instance 1 (URL: /instance/cavs/0)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs/0",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<subsystem Type=\"cavs\" Id=\"0\">"
        "    <info_parameters>"
        "        <ParameterBlock Name=\"Free Pages\">"
        "            <ParameterBlock Name=\"0\">"
        "                <EnumParameter Name=\"mem_type\">HP_MEM</EnumParameter>"
        "                <IntegerParameter Name=\"pages\">12</IntegerParameter>"
        "            </ParameterBlock>"
        "            <ParameterBlock Name=\"1\">"
        "                <EnumParameter Name=\"mem_type\">LP_MEM</EnumParameter>"
        "                <IntegerParameter Name=\"pages\">13</IntegerParameter>"
        "            </ParameterBlock>"
        "        </ParameterBlock>"
        "    </info_parameters>"
        "    <parents>"
        "        <system Type=\"SKL\" Id=\"0\"/>"
        "    </parents>"
        "    <children>"
        "        <collection Name=\"pipes\">"
        "            <!-- all pipe instances -->"
        "            <instance Type=\"pipe\" Id=\"0\"/>"
        "            <instance Type=\"pipe\" Id=\"1\"/>"
        "        </collection>"
        "        <collection Name=\"cores\">"
        "            <!-- all core instances -->"
        "            <instance Type=\"core\" Id=\"0\"/>"
        "            <instance Type=\"core\" Id=\"1\"/>"
        "        </collection>"
        "        <service_collection Name=\"services\">"
        "            <service Type=\"fwlogs\" Id=\"0\"/>"
        "        </service_collection>"
        "        <component_collection Name=\"modules\">"
        "            <!-- all module instances -->"
        "            <component Type=\"module-aec(2)\" Id=\"0\"/>"
        "            <component Type=\"module-gain(4)\" Id=\"3\"/>"
        "            <component Type=\"module-copier(1)\" Id=\"2\"/>"
        "        </component_collection>"
        "    </children>"
        "    <!-- links -->"
        "    <links>"
        "        <link Id=\"0\">"
        "            <from Type=\"module-aec(2)\" Id=\"0\" OutputId=\"1\"/>"
        "            <to Type=\"module-gain(4)\" Id=\"3\" InputId=\"0\"/>"
        "        </link>"
        "        <link Id=\"1\">"
        "            <from Type=\"module-gain(4)\" Id=\"3\" OutputId=\"2\"/>"
        "            <to Type=\"module-copier(1)\" Id=\"2\" InputId=\"0\"/>"
        "        </link>"
        "    </links>"
        "</subsystem>"
        ));
}

TEST_CASE("DebugAgent/cAVS: log type (URL: /type/cavs.fwlogs)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/type/cavs.fwlogs",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<service_type Name=\"fwlogs\">"
        "    <control_parameters>"
        "        <!-- service generic -->"
        "        <BooleanParameter Name=\"Started\"/>"
        "        <ParameterBlock Name=\"Buffering\">"
        "            <IntegerParameter Name=\"Size\" Size=\"16\" Unit=\"MegaBytes\"/>"
        "            <BooleanParameter Name=\"Circular\"/>"
        "        </ParameterBlock>"
        "        <BooleanParameter Name=\"PersistsState\"/>"
        "        <!-- service specific -->"
        "        <EnumParameter Size=\"8\" Name=\"Verbosity\">"
        "            <ValuePair Numerical=\"2\" Literal=\"Critical\"/>"
        "            <ValuePair Numerical=\"3\" Literal=\"High\"/>"
        "            <ValuePair Numerical=\"4\" Literal=\"Medium\"/>"
        "            <ValuePair Numerical=\"5\" Literal=\"Low\"/>"
        "            <ValuePair Numerical=\"6\" Literal=\"Verbose\"/>"
        "        </EnumParameter>"
        "        <BooleanParameter Name=\"ViaPTI\" Description=\"Set to 1 if PTI interface is to be used\"/>"
        "    </control_parameters>"
        "</service_type>"
        ));
}

TEST_CASE("DebugAgent/cAVS: log parameters (URL: /instance/cavs.fwlogs/0)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

    /* 1: Get log parameter, will return
    * - isStarted : false
    * - level: critical
    * - output: pti
    */

    windows::driver::IoctlFwLogsState initialLogParams = {
        false,
        windows::driver::FW_LOG_LEVEL::LOG_CRITICAL,
        windows::driver::FW_LOG_OUTPUT::OUTPUT_PTI
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
    windows::driver::IoctlFwLogsState setLogParams = {
        true,
        windows::driver::FW_LOG_LEVEL::LOG_VERBOSE,
        windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM
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
        windows::driver::IOCTL_LOG_STATE::STOPPED,
        windows::driver::FW_LOG_LEVEL::LOG_VERBOSE,
        windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM
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
        "/instance/cavs.fwlogs/0",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<service Direction=\"Outgoing\" Type=\"fwlogs\" Id=\"0\">"
        "    <parents/>"
        "    <control_parameters>"
        "        <BooleanParameter Name=\"Started\">0</BooleanParameter>"
        "        <ParameterBlock Name=\"Buffering\">"
        "            <IntegerParameter Name=\"Size\">100</IntegerParameter>"
        "            <BooleanParameter Name=\"Circular\">0</BooleanParameter>"
        "        </ParameterBlock>"
        "        <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>"
        "        <EnumParameter Name=\"Verbosity\">Critical</EnumParameter>"
        "        <BooleanParameter Name=\"ViaPTI\">1</BooleanParameter>"
        "    </control_parameters>"
        "</service>"
        ));

    /* 2: Setting log parameters ("1;Verbose;SRAM") */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0",
        HttpClientSimulator::Verb::Put,
        "<service Direction=\"Outgoing\" Type=\"fwlogs\" Id=\"0\">"
        "    <parents/>"
        "    <control_parameters>"
        "        <BooleanParameter Name=\"Started\">1</BooleanParameter>"
        "        <ParameterBlock Name=\"Buffering\">"
        "            <IntegerParameter Name=\"Size\">100</IntegerParameter>"
        "            <BooleanParameter Name=\"Circular\">0</BooleanParameter>"
        "        </ParameterBlock>"
        "        <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>"
        "        <EnumParameter Name=\"Verbosity\">Verbose</EnumParameter>"
        "        <BooleanParameter Name=\"ViaPTI\">0</BooleanParameter>"
        "    </control_parameters>"
        "</service>",
        HttpClientSimulator::Status::Ok,
        "text/html",
        "<p>Done</p>"
        ));

    /* 3: Getting log parameters again */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<service Direction=\"Outgoing\" Type=\"fwlogs\" Id=\"0\">"
        "    <parents/>"
        "    <control_parameters>"
        "        <BooleanParameter Name=\"Started\">1</BooleanParameter>"
        "        <ParameterBlock Name=\"Buffering\">"
        "            <IntegerParameter Name=\"Size\">100</IntegerParameter>"
        "            <BooleanParameter Name=\"Circular\">0</BooleanParameter>"
        "        </ParameterBlock>"
        "        <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>"
        "        <EnumParameter Name=\"Verbosity\">Verbose</EnumParameter>"
        "        <BooleanParameter Name=\"ViaPTI\">0</BooleanParameter>"
        "    </control_parameters>"
        "</service>"
        ));
}

/** The following test is based on tempos, so it is not 100% safe. These tempos are
* used to synchronize DebugAgent (and its HTTP server) and HTTP clients.
* @todo: to be reworked.
*/
TEST_CASE("DebugAgent/cAVS: debug agent shutdown while a client is consuming log")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommand(commands);

    /* 1: start log command */
    windows::driver::IoctlFwLogsState setLogParams = {
        true,
        windows::driver::FW_LOG_LEVEL::LOG_VERBOSE,
        windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM
    };
    commands.addSetLogParametersCommand(
        true,
        STATUS_SUCCESS,
        setLogParams);

    /* 2: Stop log command, will be called by the debug agent termination */
    setLogParams.started = false;
    setLogParams.level = windows::driver::FW_LOG_LEVEL::LOG_VERBOSE;
    setLogParams.output = windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM;
    commands.addSetLogParametersCommand(
        true,
        STATUS_SUCCESS,
        setLogParams);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent in another thread. It can be stopped using
    * a condition variable*/
    std::mutex debugAgentMutex;
    std::condition_variable stopDebugAgentCondVar;
    std::future<void> debugAgentFuture(std::async(std::launch::async, [&]() {

        DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

        /* Waiting for stop order */
        std::unique_lock<std::mutex> locker(debugAgentMutex);
        stopDebugAgentCondVar.wait(locker);
    }));

    /* Give some time to DebugAgent to start its http server */
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Starting log */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0",
        HttpClientSimulator::Verb::Put,
        "<service Direction=\"Outgoing\" Type=\"fwlogs\" Id=\"0\">"
        "    <parents/>"
        "    <control_parameters>"
        "        <BooleanParameter Name=\"Started\">1</BooleanParameter>"
        "        <ParameterBlock Name=\"Buffering\">"
        "            <IntegerParameter Name=\"Size\">100</IntegerParameter>"
        "            <BooleanParameter Name=\"Circular\">0</BooleanParameter>"
        "        </ParameterBlock>"
        "        <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>"
        "        <EnumParameter Name=\"Verbosity\">Verbose</EnumParameter>"
        "        <BooleanParameter Name=\"ViaPTI\">0</BooleanParameter>"
        "    </control_parameters>"
        "</service>",
        HttpClientSimulator::Status::Ok,
        "text/html",
        "<p>Done</p>"
        ));

    /* Trying to get log data in another thread after 250 ms. This should result on "resource
    * locked" http status */
    std::future<void> delayedGetLogStreamFuture(std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        HttpClientSimulator client2("localhost");
        client2.request(
            "/instance/cavs.fwlogs/0/streaming",
            HttpClientSimulator::Verb::Get,
            "",
            HttpClientSimulator::Status::Locked,
            "text/plain",
            "Resource is locked : Logging stream resource is already used.");
    }));

    /* Terminating the debug agent after 500ms in another thread, the client should be consuming
    * log at this date */
    std::future<void> debugAgentTerminationResult(std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::unique_lock<std::mutex> locker(debugAgentMutex);

        /* Terminating debug agent */
        stopDebugAgentCondVar.notify_one();
    }));

    /* Consuming log */
    try
    {
        client.request(
            "/instance/cavs.fwlogs/0/streaming",
            HttpClientSimulator::Verb::Get,
            "",
            HttpClientSimulator::Status::Ok,
            "application/vnd.ifdk-file",
            HttpClientSimulator::AnyContent);
    }
    catch (HttpClientSimulator::NetworkException &)
    {
        /* A network exception can occur here because the debug agent closes its sockets.
        * This is a normal case.
        */
    }

    /* Ensuring that the debug agent thread doesn't have thrown an exception*/
    CHECK_NOTHROW(debugAgentFuture.get());

    /* Checking that the thread that has tried to get log content although another client
     * was already getting it has obtained the expected server response */
    CHECK_NOTHROW(delayedGetLogStreamFuture.get());
}
