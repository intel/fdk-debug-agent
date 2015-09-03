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

#include "CavsTopologySample.hpp"
#include "Core/DebugAgent.hpp"
#include "Util/Uuid.hpp"
#include "Util/StringHelper.hpp"
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

using namespace debug_agent;
using namespace debug_agent::core;
using namespace debug_agent::cavs;
using namespace debug_agent::test_common;
using namespace debug_agent::util;

/** Helper function to set a module entry */
void setModuleEntry(ModuleEntry &entry, const std::string &name,
    const Uuid &uuid)
{
    /* Setting name */
    StringHelper::setStringToFixedSizeArray(entry.name, sizeof(entry.name), name);

    /* Setting GUID*/
    uuid.toOtherUuidType(entry.uuid);
}

void addInitialCommands(windows::MockedDeviceCommands &commands)
{
    /* Constructing cavs model */
    /* ----------------------- */

    std::vector<DSModuleInstanceProps> moduleInstances;
    std::vector<dsp_fw::GatewayProps> gateways;
    uint32_t maxPplCount;
    std::vector<uint32_t> pipelineIds;
    std::vector<DSPplProps> pipelines;
    std::vector<DSSchedulersInfo> schedulers;
    std::vector<ModuleEntry> modules;
    std::vector<char> fwConfig;
    std::vector<char> hwConfig;

    CavsTopologySample::createFirwareObjects(
        moduleInstances,
        gateways,
        maxPplCount,
        pipelineIds,
        pipelines,
        schedulers,
        modules,
        fwConfig,
        hwConfig);

    /* Adding initial commands */
    commands.addGetFwConfigCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        fwConfig);
    commands.addGetHwConfigCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        hwConfig);
    commands.addGetModuleEntriesCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        static_cast<uint32_t>(modules.size()),
        modules);

    /* Gateways*/
    commands.addGetGatewaysCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        static_cast<uint32_t>(gateways.size()),
        gateways);

    /* Pipelines*/
    commands.addGetPipelineListCommand(
        true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        maxPplCount,
        pipelineIds);

    for (auto &pipeline : pipelines) {
        commands.addGetPipelinePropsCommand(
            true,
            STATUS_SUCCESS,
            dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
            pipeline.id,
            pipeline);
    }

    /* Schedulers */
    uint32_t coreId = 0;
    for (auto &scheduler : schedulers) {
        commands.addGetSchedulersInfoCommand(
            true,
            STATUS_SUCCESS,
            dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
            coreId++,
            scheduler);
    }

    /* Module instances */
    for (auto &module : moduleInstances) {
        uint16_t moduleId, instanceId;
        Topology::splitModuleInstanceId(module.id, moduleId, instanceId);

        commands.addGetModuleInstancePropsCommand(
            true,
            STATUS_SUCCESS,
            dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
            moduleId,
            instanceId,
            module);
    }
}

TEST_CASE("DebugAgent/cAVS: topology")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system type*/
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


    /* 2: Getting system instance */
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


    /* 3 Getting subsystem type*/
    CHECK_NOTHROW(client.request(
        "/type/cavs",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<subsystem_type Name=\"cavs\">\n"
        "    <description>cAVS subsystem</description>\n"
        "    <characteristics>\n"
        "        <characteristic Name=\"Firmware version\">1.2.3.4</characteristic>\n"
        "        <characteristic Name=\"Maximum number of pipelines instances\">10"
        "</characteristic>\n"
        "        <characteristic Name=\"Current total number of module entries loaded "
        "into the DSP\">7</characteristic>\n"
        "        <characteristic Name=\"Number of cores\">1</characteristic>\n"
        "        <characteristic Name=\"Total number of DMA gateways\">5"
        "</characteristic>\n"
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
        "            <component_type Name=\"module.copier\"/>\n"
        "            <component_type Name=\"module.aec\"/>\n"
        "            <component_type Name=\"module.gain\"/>\n"
        "            <component_type Name=\"module.ns\"/>\n"
        "            <component_type Name=\"module.mixin\"/>\n"
        "            <component_type Name=\"module.src\"/>\n"
        "            <component_type Name=\"module.mixout\"/>\n"
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
        "        <component_type Name=\"module.copier\"/>\n"
        "        <component_type Name=\"module.aec\"/>\n"
        "        <component_type Name=\"module.gain\"/>\n"
        "        <component_type Name=\"module.ns\"/>\n"
        "        <component_type Name=\"module.mixin\"/>\n"
        "        <component_type Name=\"module.src\"/>\n"
        "        <component_type Name=\"module.mixout\"/>\n"
        "    </categories>\n"
        "</subsystem_type>\n"
        ));

    /* 4: Getting subsystem instance collection*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<subsystem_collection>\n"
        "    <subsystem Id=\"0\" Type=\"cavs\">\n"
        "        <info_parameters/>\n"
        "        <control_parameters/>\n"
        "        <parents>\n"
        "            <system Id=\"0\" Type=\"bxtn\"/>\n"
        "        </parents>\n"
        "        <children>\n"
        "            <collection Name=\"pipes\">\n"
        "                <instance Id=\"1\" Type=\"pipe\"/>\n"
        "                <instance Id=\"2\" Type=\"pipe\"/>\n"
        "                <instance Id=\"3\" Type=\"pipe\"/>\n"
        "                <instance Id=\"4\" Type=\"pipe\"/>\n"
        "            </collection>\n"
        "            <collection Name=\"cores\">\n"
        "                <instance Id=\"0\" Type=\"core\"/>\n"
        "            </collection>\n"
        "            <collection Name=\"tasks\">\n"
        "                <instance Id=\"1\" Type=\"task\"/>\n"
        "                <instance Id=\"2\" Type=\"task\"/>\n"
        "                <instance Id=\"3\" Type=\"task\"/>\n"
        "                <instance Id=\"9\" Type=\"task\"/>\n"
        "                <instance Id=\"4\" Type=\"task\"/>\n"
        "                <instance Id=\"5\" Type=\"task\"/>\n"
        "                <instance Id=\"6\" Type=\"task\"/>\n"
        "            </collection>\n"
        "            <component_collection Name=\"gateways\">\n"
        "                <component Id=\"1\" Type=\"hda-host-in-gateway\"/>\n"
        "                <component Id=\"2\" Type=\"hda-host-in-gateway\"/>\n"
        "                <component Id=\"1\" Type=\"hda-link-out-gateway\"/>\n"
        "                <component Id=\"1\" Type=\"dmic-link-in-gateway\"/>\n"
        "                <component Id=\"1\" Type=\"hda-host-out-gateway\"/>\n"
        "            </component_collection>\n"
        "            <component_collection Name=\"modules\">\n"
        "                <component Id=\"1\" Type=\"module.copier\"/>\n"
        "                <component Id=\"2\" Type=\"module.aec\"/>\n"
        "                <component Id=\"5\" Type=\"module.aec\"/>\n"
        "                <component Id=\"1\" Type=\"module.gain\"/>\n"
        "                <component Id=\"4\" Type=\"module.gain\"/>\n"
        "                <component Id=\"5\" Type=\"module.gain\"/>\n"
        "                <component Id=\"9\" Type=\"module.gain\"/>\n"
        "                <component Id=\"2\" Type=\"module.ns\"/>\n"
        "                <component Id=\"6\" Type=\"module.ns\"/>\n"
        "                <component Id=\"1\" Type=\"module.mixin\"/>\n"
        "                <component Id=\"0\" Type=\"module.src\"/>\n"
        "                <component Id=\"3\" Type=\"module.mixout\"/>\n"
        "            </component_collection>\n"
        "        </children>\n"
        "        <inputs/>\n"
        "        <outputs/>\n"
        "        <links>\n"
        "            <link>\n"
        "                <from Id=\"1\" OutputId=\"0\" Type=\"hda-host-in-gateway\"/>\n"
        "                <to Id=\"1\" InputId=\"0\" Type=\"module.copier\"/>\n"
        "            </link>\n"
        "            <link>\n"
        "                <from Id=\"1\" OutputId=\"0\" Type=\"dmic-link-in-gateway\"/>\n"
        "                <to Id=\"1\" InputId=\"0\" Type=\"module.gain\"/>\n"
        "            </link>\n"
        "            <link>\n"
        "                <from Id=\"2\" OutputId=\"0\" Type=\"hda-host-in-gateway\"/>\n"
        "                <to Id=\"4\" InputId=\"0\" Type=\"module.gain\"/>\n"
        "            </link>\n"
        "            <link>\n"
        "                <from Id=\"9\" OutputId=\"0\" Type=\"module.gain\"/>\n"
        "                <to Id=\"1\" InputId=\"0\" Type=\"hda-link-out-gateway\"/>\n"
        "            </link>\n"
        "            <link>\n"
        "                <from Id=\"3\" OutputId=\"0\" Type=\"module.mixout\"/>\n"
        "                <to Id=\"1\" InputId=\"0\" Type=\"hda-host-out-gateway\"/>\n"
        "            </link>\n"
        "        </links>\n"
        "    </subsystem>\n"
        "</subsystem_collection>\n"
        ));

    /* 4: Getting one subsystem instance*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs/0",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        "<subsystem Id=\"0\" Type=\"cavs\">\n"
        "    <info_parameters/>\n"
        "    <control_parameters/>\n"
        "    <parents>\n"
        "        <system Id=\"0\" Type=\"bxtn\"/>\n"
        "    </parents>\n"
        "    <children>\n"
        "        <collection Name=\"pipes\">\n"
        "            <instance Id=\"1\" Type=\"pipe\"/>\n"
        "            <instance Id=\"2\" Type=\"pipe\"/>\n"
        "            <instance Id=\"3\" Type=\"pipe\"/>\n"
        "            <instance Id=\"4\" Type=\"pipe\"/>\n"
        "        </collection>\n"
        "        <collection Name=\"cores\">\n"
        "            <instance Id=\"0\" Type=\"core\"/>\n"
        "        </collection>\n"
        "        <collection Name=\"tasks\">\n"
        "            <instance Id=\"1\" Type=\"task\"/>\n"
        "            <instance Id=\"2\" Type=\"task\"/>\n"
        "            <instance Id=\"3\" Type=\"task\"/>\n"
        "            <instance Id=\"9\" Type=\"task\"/>\n"
        "            <instance Id=\"4\" Type=\"task\"/>\n"
        "            <instance Id=\"5\" Type=\"task\"/>\n"
        "            <instance Id=\"6\" Type=\"task\"/>\n"
        "        </collection>\n"
        "        <component_collection Name=\"gateways\">\n"
        "            <component Id=\"1\" Type=\"hda-host-in-gateway\"/>\n"
        "            <component Id=\"2\" Type=\"hda-host-in-gateway\"/>\n"
        "            <component Id=\"1\" Type=\"hda-link-out-gateway\"/>\n"
        "            <component Id=\"1\" Type=\"dmic-link-in-gateway\"/>\n"
        "            <component Id=\"1\" Type=\"hda-host-out-gateway\"/>\n"
        "        </component_collection>\n"
        "        <component_collection Name=\"modules\">\n"
        "            <component Id=\"1\" Type=\"module.copier\"/>\n"
        "            <component Id=\"2\" Type=\"module.aec\"/>\n"
        "            <component Id=\"5\" Type=\"module.aec\"/>\n"
        "            <component Id=\"1\" Type=\"module.gain\"/>\n"
        "            <component Id=\"4\" Type=\"module.gain\"/>\n"
        "            <component Id=\"5\" Type=\"module.gain\"/>\n"
        "            <component Id=\"9\" Type=\"module.gain\"/>\n"
        "            <component Id=\"2\" Type=\"module.ns\"/>\n"
        "            <component Id=\"6\" Type=\"module.ns\"/>\n"
        "            <component Id=\"1\" Type=\"module.mixin\"/>\n"
        "            <component Id=\"0\" Type=\"module.src\"/>\n"
        "            <component Id=\"3\" Type=\"module.mixout\"/>\n"
        "        </component_collection>\n"
        "    </children>\n"
        "    <inputs/>\n"
        "    <outputs/>\n"
        "    <links>\n"
        "        <link>\n"
        "            <from Id=\"1\" OutputId=\"0\" Type=\"hda-host-in-gateway\"/>\n"
        "            <to Id=\"1\" InputId=\"0\" Type=\"module.copier\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"1\" OutputId=\"0\" Type=\"dmic-link-in-gateway\"/>\n"
        "            <to Id=\"1\" InputId=\"0\" Type=\"module.gain\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"2\" OutputId=\"0\" Type=\"hda-host-in-gateway\"/>\n"
        "            <to Id=\"4\" InputId=\"0\" Type=\"module.gain\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"9\" OutputId=\"0\" Type=\"module.gain\"/>\n"
        "            <to Id=\"1\" InputId=\"0\" Type=\"hda-link-out-gateway\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"3\" OutputId=\"0\" Type=\"module.mixout\"/>\n"
        "            <to Id=\"1\" InputId=\"0\" Type=\"hda-host-out-gateway\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"1\" OutputId=\"1\" Type=\"module.copier\"/>\n"
        "            <to Id=\"2\" InputId=\"1\" Type=\"module.aec\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"2\" OutputId=\"1\" Type=\"module.aec\"/>\n"
        "            <to Id=\"5\" InputId=\"1\" Type=\"module.gain\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"4\" OutputId=\"1\" Type=\"module.gain\"/>\n"
        "            <to Id=\"5\" InputId=\"1\" Type=\"module.aec\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"5\" OutputId=\"1\" Type=\"module.aec\"/>\n"
        "            <to Id=\"6\" InputId=\"1\" Type=\"module.ns\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"1\" OutputId=\"1\" Type=\"module.mixin\"/>\n"
        "            <to Id=\"0\" InputId=\"1\" Type=\"module.src\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"0\" OutputId=\"1\" Type=\"module.src\"/>\n"
        "            <to Id=\"9\" InputId=\"1\" Type=\"module.gain\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"1\" OutputId=\"1\" Type=\"module.gain\"/>\n"
        "            <to Id=\"2\" InputId=\"1\" Type=\"module.ns\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"2\" OutputId=\"1\" Type=\"module.ns\"/>\n"
        "            <to Id=\"3\" InputId=\"1\" Type=\"module.mixout\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"5\" OutputId=\"1\" Type=\"module.gain\"/>\n"
        "            <to Id=\"1\" InputId=\"1\" Type=\"module.mixin\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"6\" OutputId=\"1\" Type=\"module.ns\"/>\n"
        "            <to Id=\"1\" InputId=\"2\" Type=\"module.mixin\"/>\n"
        "        </link>\n"
        "        <link>\n"
        "            <from Id=\"1\" OutputId=\"2\" Type=\"module.gain\"/>\n"
        "            <to Id=\"9\" InputId=\"2\" Type=\"module.gain\"/>\n"
        "        </link>\n"
        "    </links>\n"
        "</subsystem>\n"
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
    addInitialCommands(commands);

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
        "        <BooleanParameter Name=\"ViaPTI\" Description=\"Set to 1 if PTI interface is to "
        "be used\"/>"
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
    addInitialCommands(commands);

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
    addInitialCommands(commands);

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
