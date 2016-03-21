/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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
#include "cAVS/Windows/DeviceInjectionDriverFactory.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "cAVS/Windows/StubbedWppClientFactory.hpp"
#include "cAVS/Windows/EventHandle.hpp"
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

const std::string dataPath = "data/FunctionalTests/http/";

std::string xmlFileName(const std::string &name)
{
    return dataPath + name + ".xml";
}

std::string htmlFileName(const std::string &name)
{
    return dataPath + name + ".html";
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

/** Handle DebugAgent initial and final ioctl commands */
class DBGACommandScope
{
public:
    DBGACommandScope(windows::MockedDeviceCommands &commands) : mCommands(commands)
    {
        /* Constructing cavs model */
        /* ----------------------- */
        std::vector<dsp_fw::ModuleEntry> modules;
        Buffer fwConfig;
        Buffer hwConfig;

        CavsTopologySample::createFirmwareObjects(modules, fwConfig, hwConfig);

        /* Adding initial commands */
        mCommands.addGetFwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                        fwConfig);
        mCommands.addGetHwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                        hwConfig);
        mCommands.addGetModuleEntriesCommand(true, STATUS_SUCCESS,
                                             dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                             static_cast<uint32_t>(modules.size()), modules);
    }

    ~DBGACommandScope()
    {
        // When the probe service is destroyed, it checks if the driver service state is Idle
        mCommands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Idle);
    }

private:
    windows::MockedDeviceCommands &mCommands;
};

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

struct Fixture
{
    // When the mocked device is destructed, check that all inputs were
    // consumed.
    std::unique_ptr<windows::MockedDevice> device{
        new windows::MockedDevice([] { INFO("There are leftover test inputs."; CHECK(false);) })};
};

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: topology")
{
    /* Setting the test vector
     * ----------------------- */
    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(),
        windows::Prober::SystemEventHandlesFactory::createHandles());

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

        {"/type/cavs.probe.endpoint", "probeservice_endpoint_type"},
        {"/instance/cavs.probe.endpoint", "probeservice_endpoint_instance_collection"},
        {"/instance/cavs.probe.endpoint/0", "probeservice_endpoint_instance"},
    };

    checkUrlMap(client, urlMap);
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: internal debug urls")
{
    /* Setting the test vector
     * ----------------------- */
    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(),
        windows::Prober::SystemEventHandlesFactory::createHandles());

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
                          "(URL: /instance/cavs.module-aec/1/control_parameters)")
{
    /* Setting the test vector
     * ----------------------- */
    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);

        /* Add command for get module parameter */
        uint16_t moduleId = 1;
        uint16_t InstanceId = 1;
        commands.addGetModuleParameterCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            AecParameterId, aecControlParameterPayload);
        commands.addGetModuleParameterCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            dsp_fw::ParameterId{25}, nsControlParameterPayload);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(),
        windows::Prober::SystemEventHandlesFactory::createHandles());

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

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: A refresh error erases the previous topology ")
{
    /* Setting the test vector
     * ----------------------- */
    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);

        /* Add a bad gateway command */
        std::vector<dsp_fw::GatewayProps> emptyList;
        commands.addGetGatewaysCommand(false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                       static_cast<uint32_t>(CavsTopologySample::gatewaysCount),
                                       emptyList);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(),
        windows::Prober::SystemEventHandlesFactory::createHandles());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* Request an instance topology refresh */
    CHECK_NOTHROW(requestInstanceTopologyRefresh(client));

    /* Access to an instance */
    CHECK_NOTHROW(client.request("/instance/cavs.module-aec/2", HttpClientSimulator::Verb::Get, "",
                                 HttpClientSimulator::Status::Ok, "text/xml",
                                 HttpClientSimulator::FileContent(xmlFileName("module_instance"))));

    /* Request an instance topology refresh which will fail since commands are not in device mock */
    CHECK_NOTHROW(client.request(
        "/instance/cavs/0/refreshed", HttpClientSimulator::Verb::Post, "",
        HttpClientSimulator::Status::InternalError, "text/plain",
        HttpClientSimulator::StringContent(
            "Internal error: Cannot refresh instance model: Cannot get topology from fw: "
            "Can not retrieve gateways: Device returns an exception: OS says that io "
            "control has failed.")));

    /* Access to an instance must fail since last topology refresh has failed*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.module-aec/2", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::InternalError, "text/plain",
        HttpClientSimulator::StringContent("Internal error: Instance model is undefined.")));
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: Set module instance control parameters "
                          "(URL: /instance/cavs.module-aec/1/control_parameters)")
{
    /* Setting the test vector
     * ----------------------- */
    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);

        uint16_t moduleId = 1;
        uint16_t InstanceId = 1;
        commands.addSetModuleParameterCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            AecParameterId, aecControlParameterPayload);

        commands.addSetModuleParameterCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            dsp_fw::ParameterId{25}, nsControlParameterPayload);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(),
        windows::Prober::SystemEventHandlesFactory::createHandles());

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

TEST_CASE_METHOD(Fixture, "DebugAgent / cAVS: Getting structure of parameters(module, logs)")
{
    /* Setting the test vector
     * ----------------------- */
    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(),
        windows::Prober::SystemEventHandlesFactory::createHandles());

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

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: log parameters (URL: /instance/cavs.fwlogs/0)")
{
    /* Setting the test vector
    * ----------------------- */

    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

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
        windows::driver::IoctlFwLogsState setLogParams = {
            windows::driver::IOCTL_LOG_STATE::STARTED, windows::driver::FW_LOG_LEVEL::LOG_VERBOSE,
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
    }

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(),
        windows::Prober::SystemEventHandlesFactory::createHandles());

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    /* 1: Getting log parameters*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs.fwlogs/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("logservice_getparam_stopped"))));

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

/** The following test is based on tempos, so it is not 100% safe. These tempos are
* used to synchronize DebugAgent (and its HTTP server) and HTTP clients.
* @todo: to be reworked.
*/
TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: debug agent shutdown while a client is consuming log")
{
    /* Setting the test vector
    * ----------------------- */

    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* 1: start log command */
        windows::driver::IoctlFwLogsState setLogParams = {
            windows::driver::IOCTL_LOG_STATE::STARTED, windows::driver::FW_LOG_LEVEL::LOG_VERBOSE,
            windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM};
        commands.addSetLogParametersCommand(true, STATUS_SUCCESS, setLogParams);

        /* 2: Stop log command, will be called by the debug agent termination */
        setLogParams.started = windows::driver::IOCTL_LOG_STATE::STOPPED;
        setLogParams.level = windows::driver::FW_LOG_LEVEL::LOG_VERBOSE;
        setLogParams.output = windows::driver::FW_LOG_OUTPUT::OUTPUT_SRAM;
        commands.addSetLogParametersCommand(true, STATUS_SUCCESS, setLogParams);
    }

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(),
        windows::Prober::SystemEventHandlesFactory::createHandles());

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

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: probe service control nominal cases")
{
    static const std::size_t probeCount = 8;
    static const std::size_t enabledProbeIndex = 1;
    static_assert(enabledProbeIndex < probeCount, "wrong probe index");

    /* Setting the test vector
     * ----------------------- */

    auto &&probeEventHandles = windows::Prober::SystemEventHandlesFactory::createHandles();
    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        // 1 : Getting probe service parameters, checking that it is stopped
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Idle);

        // 2 : Getting probe endpoint parameters, checking that they are deactivated
        // -> involves no ioctl

        // 3 : Configuring probe #1 to be enabled
        // -> involves no ioctl

        // 4 : Getting probe endpoint parameters, checking that they are deactivated except the one
        //     that has been enabled
        // -> involves no ioctl

        // 5 : Starting service

        // getting current state : idle
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Idle);

        // going to Owned
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Owned);

        using Type = Prober::ProbeType;
        using Purpose = Prober::ProbePurpose;
        // setting probe configuration (probe #1 is enabled)
        cavs::Prober::SessionProbes probes = {
            {false, {0, 0, Type::Input, 0}, Purpose::Inject},
            {true, {1, 2, Type::Output, 0}, Purpose::Extract}, // Enabled
            {false, {0, 0, Type::Input, 0}, Purpose::Inject},
            {false, {0, 0, Type::Input, 0}, Purpose::Inject},
            {false, {0, 0, Type::Input, 0}, Purpose::Inject},
            {false, {0, 0, Type::Input, 0}, Purpose::Inject},
            {false, {0, 0, Type::Input, 0}, Purpose::Inject},
            {false, {0, 0, Type::Input, 0}, Purpose::Inject}};

        commands.addSetProbeConfigurationCommand(
            true, STATUS_SUCCESS, windows::Prober::toWindows(probes, probeEventHandles));

        // going to Allocated
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS,
                                         windows::driver::ProbeState::Allocated);

        // going to Get ring buffers
        windows::driver::RingBuffersDescription rb = {{nullptr, 0}, {}};
        commands.addGetRingBuffers(true, STATUS_SUCCESS, rb);

        // going to Active
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Active);

        // 6 : Getting probe service parameters, checking that it is started
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Active);

        // 7 : Extract from an enabled probe -> no ioctl.

        // 8 : Stopping service

        // getting current state : Active
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Active);

        // going to Allocated, Owned and Idle
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS,
                                         windows::driver::ProbeState::Allocated);
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Owned);
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Idle);

        // 9: Getting probe service parameters, checking that it is stopped
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Idle);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(), probeEventHandles);

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    // 1 : Getting probe service parameters, checking that it is stopped
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("probeservice_param_stopped"))));

    // 2 : Getting probe endpoint parameters, checking that they are deactivated
    for (std::size_t probeIndex = 0; probeIndex < probeCount; ++probeIndex) {
        CHECK_NOTHROW(client.request(
            "/instance/cavs.probe.endpoint/" + std::to_string(probeIndex) + "/control_parameters",
            HttpClientSimulator::Verb::Get, "", HttpClientSimulator::Status::Ok, "text/xml",
            HttpClientSimulator::FileContent(xmlFileName("probeservice_endpoint_param_disabled"))));
    }

    // 3 : Configuring probe #1 to be enabled
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe.endpoint/" + std::to_string(enabledProbeIndex) +
            "/control_parameters",
        HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_endpoint_param_enabled")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    // 4 : Getting probe endpoint parameters, checking that they are deactivated except the one that
    //     has been enabled
    for (std::size_t probeIndex = 0; probeIndex < probeCount; ++probeIndex) {
        std::string expectedFile = (probeIndex == enabledProbeIndex)
                                       ? "probeservice_endpoint_param_enabled"
                                       : "probeservice_endpoint_param_disabled";

        CHECK_NOTHROW(client.request(
            "/instance/cavs.probe.endpoint/" + std::to_string(probeIndex) + "/control_parameters",
            HttpClientSimulator::Verb::Get, "", HttpClientSimulator::Status::Ok, "text/xml",
            HttpClientSimulator::FileContent(xmlFileName(expectedFile))));
    }

    // 5 : Starting service
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_param_started")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    // 6 : Getting probe service parameters, checking that it is started
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("probeservice_param_started"))));

    // 7 : Extract from an enabled probe. TODO: currently, extraction is not implemented and the
    // result will be empty.
    CHECK_NOTHROW(client.request("/instance/cavs.probe.endpoint/1/streaming",
                                 HttpClientSimulator::Verb::Get, "",
                                 HttpClientSimulator::Status::Ok, "application/vnd.ifdk-file",
                                 HttpClientSimulator::StringContent("") /*todo*/));

    // 8 : Stopping service
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_param_stopped")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    // 9: Getting probe service parameters, checking that it is stopped
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("probeservice_param_stopped"))));
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: probe service control failure cases")
{
    /* Setting the test vector
    * ----------------------- */

    auto &&probeEventHandles = windows::Prober::SystemEventHandlesFactory::createHandles();
    {
        windows::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        // 1 : Getting probe service state, with an inconsistent driver state (Owned)
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Owned);

        // 2 : If service starting fails, it should come back to "Idle" state

        // going to Owned state and setting configuration
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Idle);
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Owned);

        using Type = Prober::ProbeType;
        using Purpose = Prober::ProbePurpose;
        cavs::Prober::SessionProbes probes = {{false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject}};

        commands.addSetProbeConfigurationCommand(
            true, STATUS_SUCCESS, windows::Prober::toWindows(probes, probeEventHandles));

        // going to Allocated, but the it fails!
        commands.addSetProbeStateCommand(false, STATUS_SUCCESS,
                                         windows::driver::ProbeState::Allocated);

        // coming back to idle : firstly getting current state (Owned), then going to Idle state
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Owned);
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Idle);

        // 3 : getting state: should be Idle
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, windows::driver::ProbeState::Idle);
    }

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    windows::DeviceInjectionDriverFactory driverFactory(
        std::move(device), std::make_unique<windows::StubbedWppClientFactory>(), probeEventHandles);

    /* Creating and starting the debug agent */
    DebugAgent debugAgent(driverFactory, HttpClientSimulator::DefaultPort, pfwConfigPath);

    /* Creating the http client */
    HttpClientSimulator client("localhost");

    // 1 : Getting probe service state, with an inconsistent driver state (Owned)
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::InternalError, "text/plain",
        HttpClientSimulator::StringContent(
            "Internal error: ParameterDispatcher: cannot get "
            "parameter value: Cannot get probe service state: Cannot get probe service state: "
            "Unexpected driver probe service state: Owned "
            "(type=cavs.probe kind=Control instance=0)")));

    // 2 : If service starting fails, it should come back to "Idle" state
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_param_started")),
        HttpClientSimulator::Status::InternalError, "text/plain",
        HttpClientSimulator::StringContent(
            "Internal error: ParameterDispatcher: cannot set "
            "control parameter value: Unable to set probe service state: Cannot set probe service "
            "state: Unable to set state to driver: TinySet error: OS says that io control has "
            "failed. (type=cavs.probe kind=Control instance=0\nvalue:\n"
            "<control_parameters>\n"
            "    <ParameterBlock Name=\"State\">\n"
            "        <BooleanParameter Name=\"Started\">1</BooleanParameter>\n"
            "    </ParameterBlock>\n"
            "</control_parameters>\n)")));

    // 3 : getting state: should be Idle
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("probeservice_param_stopped"))));
}
