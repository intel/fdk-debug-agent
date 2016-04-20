/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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
#include "Util/FileHelper.hpp"
#include "TestCommon/HttpClientSimulator.hpp"
#include "TestCommon/TestHelpers.hpp"
#include "cAVS/Linux/MockedDeviceCatchHelper.hpp"
#include "cAVS/Linux/DeviceInjectionDriverFactory.hpp"
#include "cAVS/Linux/StubbedCompressDeviceFactory.hpp"
#include "cAVS/Linux/MockedDevice.hpp"
#include "cAVS/Linux/MockedDeviceCommands.hpp"
#include "cAVS/Linux/MockedControlDeviceCommands.hpp"
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
using namespace debug_agent::cavs::linux;

using Fixture = MockedDeviceFixture;

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

const std::string dataPath = PROJECT_PATH "data/FunctionalTests/http/";

std::string xmlFileName(const std::string &name)
{
    return dataPath + name + ".xml";
}

std::string htmlFileName(const std::string &name)
{
    return dataPath + name + ".html";
}

const std::string pfwConfigPath =
    PROJECT_PATH "data/FunctionalTests/pfw/ParameterFrameworkConfigurationDBGA.xml";

/** Helper function to set a module entry */
void setModuleEntry(dsp_fw::ModuleEntry &entry, const std::string &name, const Uuid &uuid)
{
    /* Setting name */
    StringHelper::setStringToFixedSizeArray(entry.name, sizeof(entry.name), name);

    /* Setting GUID*/
    uuid.toOtherUuidType(entry.uuid);
}

/** Handle DebugAgent initial and final ioctl commands */
class DBGACommandScope
{
public:
    DBGACommandScope(linux::MockedDeviceCommands &commands) : mCommands(commands)
    {
        /* Constructing cavs model */
        /* ----------------------- */
        std::vector<dsp_fw::ModuleEntry> modules;
        Buffer fwConfig;
        Buffer hwConfig;

        CavsTopologySample::createFirmwareObjects(modules, fwConfig, hwConfig);

        /* Adding initial commands */
        mCommands.addGetFwConfigCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, fwConfig);
        mCommands.addGetHwConfigCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, hwConfig);
        mCommands.addGetModuleEntriesCommand(/*true, STATUS_SUCCESS,*/
                                             dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                             static_cast<uint32_t>(modules.size()), modules);
    }

    ~DBGACommandScope()
    {
        mCommands.addGetPerfState(Perf::State::Stopped);
        mCommands.addSetCorePowerCommand(true, 0, true);
        mCommands.addSetPerfState(Perf::State::Disabled);
    }

private:
    linux::MockedDeviceCommands &mCommands;
};

void addInstanceTopologyCommands(linux::MockedDeviceCommands &commands)
{
    std::vector<dsp_fw::ModuleInstanceProps> moduleInstances;
    std::vector<dsp_fw::GatewayProps> gateways;
    std::vector<dsp_fw::PipeLineIdType> pipelineIds;
    std::vector<dsp_fw::PplProps> pipelines;
    std::vector<dsp_fw::SchedulersInfo> schedulers;

    CavsTopologySample::createInstanceFirmwareObjects(moduleInstances, gateways, pipelineIds,
                                                      pipelines, schedulers);

    /* Gateways*/
    commands.addGetGatewaysCommand(/*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   static_cast<uint32_t>(gateways.size()), gateways);

    /* Pipelines*/
    commands.addGetPipelineListCommand(
        /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
        static_cast<uint32_t>(CavsTopologySample::maxPplCount), pipelineIds);

    for (auto &pipeline : pipelines) {
        commands.addGetPipelinePropsCommand(/*true, STATUS_SUCCESS,*/
                                            dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                            dsp_fw::PipeLineIdType{pipeline.id}, pipeline);
    }

    /* Schedulers */
    uint32_t coreId = 0;
    for (auto &scheduler : schedulers) {
        commands.addGetSchedulersInfoCommand(/*true, STATUS_SUCCESS,*/
                                             dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                             dsp_fw::CoreId{coreId++}, scheduler);
    }

    /* Module instances */
    for (auto &module : moduleInstances) {

        commands.addGetModuleInstancePropsCommand(/*true, STATUS_SUCCESS,*/
                                                  dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                                  module.id.moduleId, module.id.instanceId, module);
    }
}

static void requestInstanceTopologyRefresh(HttpClientSimulator &client)
{
    client.request("/instance/cavs/0/refreshed", HttpClientSimulator::Verb::Post, "",
                   HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent(""));
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
                           HttpClientSimulator::Status::Ok, "text/xml",
                           HttpClientSimulator::FileContent(xmlFileName(it.second)));
        } catch (std::exception &e) {
            std::stringstream stream;
            stream << "Error on url=" << it.first << " file=" << it.second << std::endl << e.what();
            INFO(stream.str());
            CHECK(false);
        }
    }
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: topology", "[topology]")
{
    /* Setting the test vector
     * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

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

        {"/type/cavs.probe", "probeservice_type"},
        {"/instance/cavs.probe", "probeservice_instance_collection"},
        {"/instance/cavs.probe/0", "probeservice_instance"},

        {"/type/cavs.perf_measurement", "perf_measurementservice_type"},
        {"/instance/cavs.perf_measurement", "perf_measurementservice_instance_collection"},
        {"/instance/cavs.perf_measurement/0", "perf_measurementservice_instance"},

        {"/type/cavs.probe.endpoint", "probeservice_endpoint_type"},
        {"/instance/cavs.probe.endpoint", "probeservice_endpoint_instance_collection"},
        {"/instance/cavs.probe.endpoint/0", "probeservice_endpoint_instance"},
    };

    checkUrlMap(client, urlMap);
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: internal debug urls", "[debug]")
{
    /* Setting the test vector
     * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    CHECK_NOTHROW(client.request(
        "/internal/modules", HttpClientSimulator::Verb::Get, "", HttpClientSimulator::Status::Ok,
        "text/html", HttpClientSimulator::FileContent(htmlFileName("internal_module_list"))));

    CHECK_NOTHROW(client.request(
        "/internal/topology", HttpClientSimulator::Verb::Get, "", HttpClientSimulator::Status::Ok,
        "text/html", HttpClientSimulator::FileContent(htmlFileName("internal_topology"))));
}

static const dsp_fw::ParameterId AecParameterId{0};

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: GET module instance control parameters "
                          "(URL: /instance/cavs.module-aec/1/control_parameters)",
                 "[module][settings]")
{
    /* Setting the test vector
     * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);

        /* Add command for get module parameter */
        uint16_t moduleId = 1;
        uint16_t InstanceId = 1;
        commands.addGetModuleParameterCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            AecParameterId, ModuleHandler::maxParameterPayloadSize, aecControlParameterPayload);
        commands.addGetModuleParameterCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            dsp_fw::ParameterId{25}, ModuleHandler::maxParameterPayloadSize,
            nsControlParameterPayload);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Request an instance topology refresh */
    CHECK_NOTHROW(requestInstanceTopologyRefresh(client));

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.module-aec/1/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("module_instance_control_params"))));
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: Set module instance control parameters "
                          "(URL: /instance/cavs.module-aec/1/control_parameters)",
                 "[module][settings]")
{
    /* Setting the test vector
     * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);

        uint16_t moduleId = 1;
        uint16_t InstanceId = 1;
        commands.addSetModuleParameterCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            AecParameterId, aecControlParameterPayload);

        commands.addSetModuleParameterCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            dsp_fw::ParameterId{25}, nsControlParameterPayload);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Request an instance topology refresh */
    CHECK_NOTHROW(requestInstanceTopologyRefresh(client));

    CHECK_NOTHROW(client.request(
        "/instance/cavs.module-aec/1/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("module_instance_control_params")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));
}

TEST_CASE_METHOD(Fixture, "DebugAgent / cAVS: Getting structure of parameters(module, logs)",
                 "[log][module][structure]")
{
    /* Setting the test vector
     * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Checking structure answers */
    std::map<std::string, std::string> systemUrlMap = {
        {"/type/cavs.module-aec/control_parameters", "module_type_control_params"}, // module
        {"/type/cavs.fwlogs/control_parameters", "logservice_control_parameter_structure"}, // logs
    };
    checkUrlMap(client, systemUrlMap);
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: log parameters (URL: /instance/cavs.fwlogs/0)", "[log]")
{
    /* Setting the test vector
    * ----------------------- */

    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);
        commands.addSetCorePowerCommand(true, 0, false);
        commands.addSetCorePowerCommand(true, 1, false);

        /* 2: Set log parameter to
        * - isStarted : true
        * - level: verbose
        * - output: sram
        */
        commands.addSetLogInfoStateCommand(true, driver::CoreMask(1 << 0 | 1 << 1), true,
                                           debug_agent::cavs::Logger::Level::Verbose);

        commands.addSetCorePowerCommand(true, 0, true);
        commands.addSetCorePowerCommand(true, 1, true);
    }

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting log parameters*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("logservice_getparam_stopped_init_values"))));

    /* 2: Setting log parameters ("1;Verbose;SRAM") */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("logservice_setparam_start")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    /* 3: Getting log parameters again */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("logservice_getparam_started"))));
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: starting same log stream twice", "[log]")
{
    /* Setting the test vector
    * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);
        commands.addSetCorePowerCommand(true, 0, false);
        commands.addSetCorePowerCommand(true, 1, false);
        commands.addSetLogInfoStateCommand(true, driver::CoreMask(1 << 0 | 1 << 1), true,
                                           debug_agent::cavs::Logger::Level::Verbose);
        commands.addSetLogInfoStateCommand(true, driver::CoreMask(1 << 0 | 1 << 1), true,
                                           debug_agent::cavs::Logger::Level::Verbose);
        commands.addSetCorePowerCommand(true, 0, true);
        commands.addSetCorePowerCommand(true, 1, true);
    }

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting log parameters*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("logservice_getparam_stopped_init_values"))));

    /* 2: Setting log parameters ("1;Verbose;SRAM") */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("logservice_setparam_start")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    /* 3: Setting log parameters, starting twice ("1;Verbose;SRAM") */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("logservice_setparam_start")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));
}

/** The following test is based on tempos, so it is not 100% safe. These tempos are
* used to synchronize DebugAgent (and its HTTP server) and HTTP clients.
* @todo: to be reworked.
*/
TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: debug agent shutdown while a client is consuming log",
                 "[log]")
{
    /* Setting the test vector
    * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);
        commands.addSetCorePowerCommand(true, 0, false);
        commands.addSetCorePowerCommand(true, 1, false);
        commands.addSetLogInfoStateCommand(true, driver::CoreMask(1 << 0 | 1 << 1), true,
                                           debug_agent::cavs::Logger::Level::Verbose);
        commands.addSetCorePowerCommand(true, 0, true);
        commands.addSetCorePowerCommand(true, 1, true);
    }

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

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
        file_helper::readAsString(xmlFileName("logservice_setparam_start")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    /* Trying to get log data in another thread after 250 ms. This should result on "resource
    * locked" http status */
    std::future<void> delayedGetLogStreamFuture(std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));

        HttpClientSimulator client2("localhost");
        client2.request("/instance/cavs.fwlogs/0/streaming", HttpClientSimulator::Verb::Get, "",
                        HttpClientSimulator::Status::Locked, "text/plain",
                        HttpClientSimulator::StringContent(
                            "Resource is locked: Logging stream resource is already used."));
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
                       HttpClientSimulator::AnyContent());
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

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: performance measurement", "[perf]")
{
    /* Setting the test vector
     * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        commands.addGetPerfState(Perf::State::Disabled);
        commands.addSetCorePowerCommand(true, 0, false);
        commands.addSetPerfState(Perf::State::Started);

        commands.addGetPerfState(Perf::State::Started);
        commands.addSetPerfState(Perf::State::Paused);

        commands.addGetPerfState(Perf::State::Paused);
        commands.addSetPerfState(Perf::State::Stopped);
        commands.addGetPerfState(Perf::State::Stopped);

        static const std::vector<dsp_fw::PerfDataItem> expectedPerfItems = {
            dsp_fw::PerfDataItem(0, 0, false, false, 1337, 42),   // Core 0
            dsp_fw::PerfDataItem(1, 0, true, false, 123456, 789), // Module 1, instance 0
            dsp_fw::PerfDataItem(0, 1, true, true, 987654, 321),  // Core 1
            dsp_fw::PerfDataItem(12, 0, false, false, 1111, 222), // Module 12, instance 0
            dsp_fw::PerfDataItem(12, 1, true, false, 3333, 444)   // Module 12, instance 1
        };
        commands.addGetGlobalPerfDataCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                             CavsTopologySample::maxModInstCount +
                                                 CavsTopologySample::dspCoreCount,
                                             expectedPerfItems);
        dsp_fw::ModuleInstanceProps props;

        props.id.moduleId = 1;
        props.id.instanceId = 0;
        props.ibs_bytes = 1;
        props.cpc = 2000;
        props.input_pins.pin_info.emplace_back();
        auto &format = props.input_pins.pin_info[0].format;
        format.sampling_frequency = dsp_fw::SamplingFrequency::FS_8000HZ;
        format.number_of_channels = 4;
        format.valid_bit_depth = 8;
        commands.addGetModuleInstancePropsCommand(/*true, STATUS_SUCCESS,*/
                                                  dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                                  props.id.moduleId, props.id.instanceId, props);
        props.id.moduleId = 12;
        props.id.instanceId = 0;
        props.ibs_bytes = 9;
        props.cpc = 10000;
        format.sampling_frequency = dsp_fw::SamplingFrequency::FS_11025HZ;
        format.number_of_channels = 12;
        format.valid_bit_depth = 16;
        commands.addGetModuleInstancePropsCommand(/*true, STATUS_SUCCESS,*/
                                                  dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                                  props.id.moduleId, props.id.instanceId, props);
        props.id.moduleId = 12;
        props.id.instanceId = 1;
        props.ibs_bytes = 17;
        props.cpc = 18000;
        format.sampling_frequency = dsp_fw::SamplingFrequency::FS_12000HZ;
        format.number_of_channels = 20;
        format.valid_bit_depth = 24;
        commands.addGetModuleInstancePropsCommand(/*true, STATUS_SUCCESS,*/
                                                  dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                                  props.id.moduleId, props.id.instanceId, props);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::move(controlDevice),
        std::make_unique<linux::StubbedCompressDeviceFactory>());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    CHECK_NOTHROW(client.request(
        "/instance/cavs.perf_measurement/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("perfservice_started")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));
    CHECK_NOTHROW(client.request(
        "/instance/cavs.perf_measurement/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("perfservice_paused")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));
    CHECK_NOTHROW(client.request(
        "/instance/cavs.perf_measurement/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("perfservice_stopped")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    CHECK_NOTHROW(client.request(
        "/instance/cavs.perf_measurement/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("perfservice_stopped"))));

    CHECK_NOTHROW(
        client.request("/instance/cavs.perf_measurement/0/perf", HttpClientSimulator::Verb::Get, "",
                       HttpClientSimulator::Status::Ok, "text/xml",
                       HttpClientSimulator::FileContent(xmlFileName("perfservice_data"))));
}
