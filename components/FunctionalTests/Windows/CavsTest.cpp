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

static const util::Buffer aecControlParameterPayload = {
    0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
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
    0x00, 0xAA};
static const util::Buffer nsControlParameterPayload = {
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF1, 0xFF, 0xF1,
    0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/** @return the file content as string */
std::string fileContent(const std::string &name)
{
    std::string fileName = "data/FunctionalTests/http/" + name;

    std::ifstream file(fileName);
    if (!file) { /* Using stream bool operator */
        throw std::logic_error("Unable to open file: " + fileName);
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (file.bad()) {
        throw std::logic_error("Error while reading file: " + fileName);
    }

    return StringHelper::trim(content) + "\n"; /* Poco xml library puts a '\n' on the last line. */
}

std::string xmlFile(const std::string &name)
{
    return fileContent(name + ".xml");
}

std::string htmlFile(const std::string &name)
{
    return fileContent(name + ".html");
}

const std::string &pfwConfigPath =
    "data/FunctionalTests/pfw/ParameterFrameworkConfigurationDBGA.xml";

/** Helper function to set a module entry */
void setModuleEntry(dsp_fw::ModuleEntry &entry, const std::string &name, const Uuid &uuid)
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
    std::vector<dsp_fw::ModuleEntry> modules;
    Buffer fwConfig;
    Buffer hwConfig;

    CavsTopologySample::createFirmwareObjects(modules, fwConfig, hwConfig);

    /* Adding initial commands */
    commands.addGetFwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   fwConfig);
    commands.addGetHwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   hwConfig);
    commands.addGetModuleEntriesCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                        static_cast<uint32_t>(modules.size()), modules);
}

void addInstanceTopologyCommands(windows::MockedDeviceCommands &commands)
{
    std::vector<dsp_fw::ModuleInstanceProps> moduleInstances;
    std::vector<dsp_fw::GatewayProps> gateways;
    std::vector<dsp_fw::PipeLineIdType> pipelineIds;
    std::vector<dsp_fw::PplProps> pipelines;
    std::vector<dsp_fw::SchedulersInfo> schedulers;

    CavsTopologySample::createInstanceFirmwareObjects(moduleInstances, gateways, pipelineIds,
                                                      pipelines, schedulers);

    /* Gateways*/
    commands.addGetGatewaysCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   static_cast<uint32_t>(gateways.size()), gateways);

    /* Pipelines*/
    commands.addGetPipelineListCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                       static_cast<uint32_t>(CavsTopologySample::maxPplCount),
                                       pipelineIds);

    for (auto &pipeline : pipelines) {
        commands.addGetPipelinePropsCommand(true, STATUS_SUCCESS,
                                            dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                            dsp_fw::PipeLineIdType{pipeline.id}, pipeline);
    }

    /* Schedulers */
    uint32_t coreId = 0;
    for (auto &scheduler : schedulers) {
        commands.addGetSchedulersInfoCommand(true, STATUS_SUCCESS,
                                             dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                             dsp_fw::CoreId{coreId++}, scheduler);
    }

    /* Module instances */
    for (auto &module : moduleInstances) {

        commands.addGetModuleInstancePropsCommand(true, STATUS_SUCCESS,
                                                  dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                                  module.id.moduleId, module.id.instanceId, module);
    }
}

static void requestInstanceTopologyRefresh(HttpClientSimulator &client)
{
    client.request("/instance/cavs/0/refreshed", HttpClientSimulator::Verb::Post, "",
                   HttpClientSimulator::Status::Ok, "", "");
}

/* Check that urls contained in the supplied map match the expected xml result
 *
 * Key of the map: the url
 * Value of the map: a file that contains the expected xml
 */
static void checkUrlMap(HttpClientSimulator &client,
                        const std::map<std::string, std::string> &urlMap)
{
    for (auto it : urlMap) {
        try {
            client.request(it.first, HttpClientSimulator::Verb::Get, "",
                           HttpClientSimulator::Status::Ok, "text/xml", xmlFile(it.second));
        } catch (...) {
            /* Proving some error information because catch doesn't display them. */
            std::cout << "------------------------------------------------------------" << std::endl
                      << "Error on url=" << it.first << " file=" << it.second << std::endl
                      << "------------------------------------------------------------"
                      << std::endl;
            CHECK_NOTHROW(throw);
        }
    }
}

TEST_CASE("DebugAgent/cAVS: topology")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device = std::make_unique<windows::MockedDevice>();

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);
    /* Adding topology command */
    addInstanceTopologyCommands(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* System type and instance are available before refresh */
    std::map<std::string, std::string> systemUrlMap = {
        {"/type", "system_type"}, {"/instance", "system_instance"},
    };
    checkUrlMap(client, systemUrlMap);

    /* Request an instance topology refresh */
    CHECK_NOTHROW(requestInstanceTopologyRefresh(client));

    /* Testing urls that depend of topology retrieval */
    std::map<std::string, std::string> urlMap = {
        {"/type", "system_type"},
        {"/instance", "system_instance"},

        {"/type/cavs", "subsystem_type"},
        {"/instance/cavs", "subsystem_instance_collection"},
        {"/instance/cavs/0", "subsystem_instance"},

        {"/type/cavs.pipe", "pipe_type"},
        {"/instance/cavs.pipe", "pipe_instance_collection"},
        {"/instance/cavs.pipe/1", "pipe_instance"},

        {"/type/cavs.task", "task_type"},
        {"/instance/cavs.task", "task_instance_collection"},
        {"/instance/cavs.task/1", "task_instance"},

        {"/type/cavs.core", "core_type"},
        {"/instance/cavs.core", "core_instance_collection"},
        {"/instance/cavs.core/0", "core_instance"},

        {"/type/cavs.module-aec", "module_type"},
        {"/instance/cavs.module-aec", "module_instance_collection"},
        {"/instance/cavs.module-aec/2", "module_instance"},

        {"/type/cavs.hda-host-out-gateway", "gateway_type"},
        {"/instance/cavs.hda-host-out-gateway", "gateway_instance_collection"},
        {"/instance/cavs.hda-host-out-gateway/1", "gateway_instance"},

        {"/type/cavs.fwlogs", "logservice_type"},
        {"/instance/cavs.fwlogs", "logservice_instance_collection"},
        {"/instance/cavs.fwlogs/0", "logservice_instance"},
    };

    checkUrlMap(client, urlMap);
}

TEST_CASE("DebugAgent/cAVS: internal debug urls")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device = std::make_unique<windows::MockedDevice>();

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);
    /* Adding topology command */
    addInstanceTopologyCommands(commands);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    CHECK_NOTHROW(client.request("/internal/modules", HttpClientSimulator::Verb::Get, "",
                                 HttpClientSimulator::Status::Ok, "text/html",
                                 htmlFile("internal_module_list")));

    CHECK_NOTHROW(client.request("/internal/topology", HttpClientSimulator::Verb::Get, "",
                                 HttpClientSimulator::Status::Ok, "text/html",
                                 htmlFile("internal_topology")));
}

/** Test a parameter id over 32 bit. */
static const dsp_fw::ParameterId AecParameterId{1LL << 32};

TEST_CASE("DebugAgent/cAVS: GET module instance control parameters "
          "(URL: /instance/cavs.module-aec/1/control_parameters)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device = std::make_unique<windows::MockedDevice>();

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);
    /* Adding topology command */
    addInstanceTopologyCommands(commands);

    /* Add command for get module parameter */
    uint16_t moduleId = 1;
    uint16_t InstanceId = 1;
    commands.addGetModuleParameterCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                          moduleId, InstanceId, AecParameterId,
                                          aecControlParameterPayload);
    commands.addGetModuleParameterCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                          moduleId, InstanceId, dsp_fw::ParameterId{25},
                                          nsControlParameterPayload);
    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Request an instance topology refresh */
    CHECK_NOTHROW(requestInstanceTopologyRefresh(client));

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.module-aec/1/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml", xmlFile("module_instance_control_params")));
}

TEST_CASE("DebugAgent/cAVS: A refresh error erases the previous topology ")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device = std::make_unique<windows::MockedDevice>();

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);
    /* Adding topology command */
    addInstanceTopologyCommands(commands);

    /* Add a bad gateway command */
    std::vector<dsp_fw::GatewayProps> emptyList;
    commands.addGetGatewaysCommand(false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   static_cast<uint32_t>(CavsTopologySample::gatewaysCount),
                                   emptyList);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Request an instance topology refresh */
    CHECK_NOTHROW(requestInstanceTopologyRefresh(client));

    /* Access to an instance */
    CHECK_NOTHROW(client.request("/instance/cavs.module-aec/2", HttpClientSimulator::Verb::Get, "",
                                 HttpClientSimulator::Status::Ok, "text/xml",
                                 xmlFile("module_instance")));

    /* Request an instance topology refresh which will fail since commands are not in device mock */
    CHECK_NOTHROW(client.request(
        "/instance/cavs/0/refreshed", HttpClientSimulator::Verb::Post, "",
        HttpClientSimulator::Status::InternalError, "text/plain",
        "Internal error: Cannot refresh instance model: Cannot get topology from fw: "
        "Can not retrieve gateways: Device returns an exception: OS says that io "
        "control has failed."));

    /* Access to an instance must fail since last topology refresh has failed*/
    CHECK_NOTHROW(client.request("/instance/cavs.module-aec/2", HttpClientSimulator::Verb::Get, "",
                                 HttpClientSimulator::Status::InternalError, "text/plain",
                                 "Internal error: Instance model is undefined."));
}

TEST_CASE("DebugAgent/cAVS: Set module instance control parameters "
          "(URL: /instance/cavs.module-aec/1/control_parameters)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device = std::make_unique<windows::MockedDevice>();

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);
    /* Adding topology command */
    addInstanceTopologyCommands(commands);
    /* Add command to set module parameter */
    uint16_t moduleId = 1;
    uint16_t InstanceId = 1;
    commands.addSetModuleParameterCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                          moduleId, InstanceId, AecParameterId,
                                          aecControlParameterPayload);

    commands.addSetModuleParameterCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                          moduleId, InstanceId, dsp_fw::ParameterId{25},
                                          nsControlParameterPayload);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Request an instance topology refresh */
    CHECK_NOTHROW(requestInstanceTopologyRefresh(client));

    CHECK_NOTHROW(client.request(
        "/instance/cavs.module-aec/1/control_parameters", HttpClientSimulator::Verb::Put,
        xmlFile("module_instance_control_params"), HttpClientSimulator::Status::Ok, "", ""));
}

TEST_CASE("DebugAgent/cAVS: module type control parameters "
          "(URL: /type/cavs.module-aec/1/control_parameters)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device = std::make_unique<windows::MockedDevice>();

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/type/cavs.module-aec/1/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml", xmlFile("module_type_control_params")));
}

TEST_CASE("DebugAgent/cAVS: log parameters (URL: /instance/cavs.fwlogs/0)")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device = std::make_unique<windows::MockedDevice>();

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
        windows::driver::IOCTL_LOG_STATE::STOPPED, windows::driver::FW_LOG_LEVEL::LOG_CRITICAL,
        windows::driver::FW_LOG_OUTPUT::OUTPUT_PTI};
    commands.addGetLogParametersCommand(true, STATUS_SUCCESS, initialLogParams);

    /* 2: Set log parameter to
    * - isStarted : true
    * - level: verbose
    * - output: sram
    */
    windows::driver::IoctlFwLogsState setLogParams = {windows::driver::IOCTL_LOG_STATE::STARTED,
                                                      windows::driver::FW_LOG_LEVEL::LOG_VERBOSE,
                                                      windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM};
    commands.addSetLogParametersCommand(true, STATUS_SUCCESS, setLogParams);

    /* 3: Get log parameter , will return
    * - isStarted : true
    * - level: verbose
    * - output: sram
    */
    commands.addGetLogParametersCommand(true, STATUS_SUCCESS, setLogParams);

    /** Adding a successful set log parameters command, this is called by the System class
    * destructor to stop log */
    setLogParams = {windows::driver::IOCTL_LOG_STATE::STOPPED,
                    windows::driver::FW_LOG_LEVEL::LOG_VERBOSE,
                    windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM};
    commands.addSetLogParametersCommand(true, STATUS_SUCCESS, setLogParams);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(std::make_unique<windows::StubbedWppClientFactory>()));

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting log parameters*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml", xmlFile("logservice_getparam_stopped")));

    /* 2: Setting log parameters ("1;Verbose;SRAM") */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Put,
        xmlFile("logservice_setparam_start"), HttpClientSimulator::Status::Ok, "", ""));

    /* 3: Getting log parameters again */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml", xmlFile("logservice_getparam_started")));
}

/** The following test is based on tempos, so it is not 100% safe. These tempos are
* used to synchronize DebugAgent (and its HTTP server) and HTTP clients.
* @todo: to be reworked.
*/
TEST_CASE("DebugAgent/cAVS: debug agent shutdown while a client is consuming log")
{
    /* Creating the mocked device */
    std::unique_ptr<windows::MockedDevice> device = std::make_unique<windows::MockedDevice>();

    /* Setting the test vector
    * ----------------------- */

    windows::MockedDeviceCommands commands(*device);

    /* Adding initial commands */
    addInitialCommands(commands);

    /* 1: start log command */
    windows::driver::IoctlFwLogsState setLogParams = {windows::driver::IOCTL_LOG_STATE::STARTED,
                                                      windows::driver::FW_LOG_LEVEL::LOG_VERBOSE,
                                                      windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM};
    commands.addSetLogParametersCommand(true, STATUS_SUCCESS, setLogParams);

    /* 2: Stop log command, will be called by the debug agent termination */
    setLogParams.started = windows::driver::IOCTL_LOG_STATE::STOPPED;
    setLogParams.level = windows::driver::FW_LOG_LEVEL::LOG_VERBOSE;
    setLogParams.output = windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM;
    commands.addSetLogParametersCommand(true, STATUS_SUCCESS, setLogParams);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(std::make_unique<windows::StubbedWppClientFactory>()));

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
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Put,
        xmlFile("logservice_setparam_start"), HttpClientSimulator::Status::Ok, "", ""));

    /* Trying to get log data in another thread after 250 ms. This should result on "resource
    * locked" http status */
    std::future<void> delayedGetLogStreamFuture(std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        HttpClientSimulator client2("localhost");
        client2.request("/instance/cavs.fwlogs/0/streaming", HttpClientSimulator::Verb::Get, "",
                        HttpClientSimulator::Status::Locked, "text/plain",
                        "Resource is locked: Logging stream resource is already used.");
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
    try {
        client.request("/instance/cavs.fwlogs/0/streaming", HttpClientSimulator::Verb::Get, "",
                       HttpClientSimulator::Status::Ok, "application/vnd.ifdk-file",
                       HttpClientSimulator::AnyContent);
    } catch (HttpClientSimulator::NetworkException &) {
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
