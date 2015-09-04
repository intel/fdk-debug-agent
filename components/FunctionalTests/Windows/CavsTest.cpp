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
#include <fstream>

using namespace debug_agent;
using namespace debug_agent::core;
using namespace debug_agent::cavs;
using namespace debug_agent::test_common;
using namespace debug_agent::util;

static const std::vector<uint8_t> aecControlParameterPayload =
{ 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
0x01, 0x00, 0xF1, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF1, 0xFF, 0xF1, 0xFF,
0xF1, 0xFF, 0xF1, 0xFF, 0x00, 0x00, 0xF1, 0xFF, 0xF1, 0xFF, 0x00, 0x00, 0xF1, 0xFF, 0x00, 0x00,
0xF4, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80,
0x00, 0x80, 0xF1, 0xFF, 0xF1, 0xFF, 0xF1, 0xFF, 0xF1, 0xFF, 0xF1, 0xFF, 0xF1, 0xFF, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xAA };
static const std::vector<uint8_t> nsControlParameterPayload =
{ 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF1, 0xFF, 0xF1, 0xFF, 0x01, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/** @return the xml file content as string */
std::string xmlFile(const std::string &name)
{
    std::string fileName = "data/FunctionalTests/" + name + ".xml";

    std::ifstream file(fileName);
    if (!file.is_open()) {
        throw std::logic_error("Unknown xml file: " + fileName);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    if (file.bad()) {
        throw std::logic_error("Error while reading xml file: " + fileName);
    }

    return StringHelper::trim(content) + "\n"; /* Poco xml library puts a '\n' on the last line. */
}

const std::string& pfwConfigPath = "data/ParameterFrameworkConfigurationDBGA.xml";

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
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Key: the url
     * Value: a file that contains the expected xml
     */
    std::map<std::string, std::string> urlMap = {
        { "/type",                              "system_type" },
        { "/instance",                          "system_instance" },
        { "/type/cavs",                         "subsystem_type" },
        { "/instance/cavs",                     "subsystem_instance_collection" },
        { "/instance/cavs/0",                   "subsystem_instance" },
        { "/type/cavs.pipe",                    "pipe_type" },
        { "/type/cavs.task",                    "task_type" },
        { "/type/cavs.core",                    "core_type" },
        { "/type/cavs.module.aec",              "module_type" },
        { "/type/cavs.hda-host-out-gateway",    "gateway_type" }
    };

    for (auto it : urlMap) {
        try{
            client.request(
                it.first,
                HttpClientSimulator::Verb::Get,
                "",
                HttpClientSimulator::Status::Ok,
                "text/xml",
                xmlFile(it.second));
        }
        catch (...)
        {
            /* Proving some error information because catch doesn't display them. */
            std::cout
                << "------------------------------------------------------------" << std::endl
                << "Error on url=" << it.first << " file=" << it.second << std::endl
                << "------------------------------------------------------------" << std::endl;
            CHECK_NOTHROW(throw);
        }
    }
}

TEST_CASE("DebugAgent/cAVS: GET module instance control parameters "
    "(URL: /instance/cavs.module-aec/1/control_parameters)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);
    /* Add command for get module parameter */
    uint16_t moduleId = 1;
    uint16_t InstanceId = 1;
    uint32_t ParamId = 0;
    commands.addGetModuleParameterCommand(true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        moduleId, InstanceId, ParamId,
        aecControlParameterPayload);
    commands.addGetModuleParameterCommand(true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        moduleId, InstanceId, 25,
        nsControlParameterPayload);
    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.module-aec/1/control_parameters",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        xmlFile("module_instance_control_params")
        ));
}

TEST_CASE("DebugAgent/cAVS: Set module instance control parameters "
    "(URL: /instance/cavs.module-aec/1/control_parameters)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);
    /* Add command to set module parameter */
    uint16_t moduleId = 1;
    uint16_t InstanceId = 1;
    uint32_t ParamId = 0;
    commands.addSetModuleParameterCommand(true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        moduleId, InstanceId, ParamId,
        aecControlParameterPayload);

    commands.addSetModuleParameterCommand(true,
        STATUS_SUCCESS,
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS,
        moduleId, InstanceId, 25,
        nsControlParameterPayload);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.module-aec/1/control_parameters",
        HttpClientSimulator::Verb::Put,
        xmlFile("module_instance_control_params"),
        HttpClientSimulator::Status::Ok,
        "text/html",
        "<p>Done</p>"
        ));
}

TEST_CASE("DebugAgent/cAVS: module type control parameters "
    "(URL: /type/cavs.module-aec/1/control_parameters)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device(new windows::MockedDevice());

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(std::move(device),
        std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/type/cavs.module-aec/1/control_parameters",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        xmlFile("module_type_control_params")
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
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/type/cavs.fwlogs",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        xmlFile("logservice_type")));
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
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting log parameters*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0",
        HttpClientSimulator::Verb::Get,
        "",
        HttpClientSimulator::Status::Ok,
        "text/xml",
        xmlFile("logservice_getparam_stopped")));

    /* 2: Setting log parameters ("1;Verbose;SRAM") */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0",
        HttpClientSimulator::Verb::Put,
        xmlFile("logservice_setparam_start"),
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
        xmlFile("logservice_getparam_started")));
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

        DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

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
        xmlFile("logservice_setparam_start"),
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
