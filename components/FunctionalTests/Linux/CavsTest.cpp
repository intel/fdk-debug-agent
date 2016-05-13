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
#include "cAVS/Linux/SyncWait.hpp"
#include "cAVS/Linux/MockedDeviceCatchHelper.hpp"
#include "cAVS/Linux/DeviceInjectionDriverFactory.hpp"
#include "cAVS/Linux/StubbedCompressDeviceFactory.hpp"
#include "cAVS/Linux/MockedCompressDeviceFactory.hpp"
#include "cAVS/Linux/MockedDevice.hpp"
#include "cAVS/Linux/MockedDeviceCommands.hpp"
#include "cAVS/Linux/MockedControlDeviceCommands.hpp"
#include "cAVS/Linux/ControlDeviceTypes.hpp"
#include "cAVS/Linux/Prober.hpp"
#include "cAVS/DspFw/Probe.hpp"
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
using debug_agent::cavs::linux::SyncWait;

using Fixture = linux::MockedDeviceFixture;

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

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: subsystem info parameters", "[subsystem],[info]")
{
    /* Setting the test vector
     * ----------------------- */
    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        /* Adding topology command */
        addInstanceTopologyCommands(commands);

        commands.addGetPerfState(Perf::State::Disabled);
        commands.addSetCorePowerCommand(true, 0, false);
        commands.addSetPerfState(Perf::State::Started);

        static const std::vector<dsp_fw::PerfDataItem> expectedPerfItems = {
            dsp_fw::PerfDataItem(0, 0, false, false, 1337, 42),   // Core 0
            dsp_fw::PerfDataItem(1, 0, true, false, 123456, 789), // Module 1, instance 0
            dsp_fw::PerfDataItem(9, 0, false, false, 1111, 222),  // Module 9, instance 0
            dsp_fw::PerfDataItem(9, 1, true, false, 3333, 444),   // Module 9, instance 1
            dsp_fw::PerfDataItem(9, 2, true, true, 5555, 666)     // Module 9, instance 2, removed
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
        props.id.moduleId = 9;
        props.id.instanceId = 0;
        props.ibs_bytes = 9;
        props.cpc = 10000;
        format.sampling_frequency = dsp_fw::SamplingFrequency::FS_11025HZ;
        format.number_of_channels = 12;
        format.valid_bit_depth = 16;
        commands.addGetModuleInstancePropsCommand(/*true, STATUS_SUCCESS,*/
                                                  dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                                  props.id.moduleId, props.id.instanceId, props);
        props.id.moduleId = 9;
        props.id.instanceId = 1;
        props.ibs_bytes = 17;
        props.cpc = 18000;
        format.sampling_frequency = dsp_fw::SamplingFrequency::FS_12000HZ;
        format.number_of_channels = 20;
        format.valid_bit_depth = 24;
        commands.addGetModuleInstancePropsCommand(/*true, STATUS_SUCCESS,*/
                                                  dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                                  props.id.moduleId, props.id.instanceId, props);

        Buffer globalMemoryState = {/* Tag for LPSRAM_STATE: 0 */
                                    0, 0, 0, 0,
                                    /* Length: (1+(1+1)+(1+0.5)) = 4.5 */
                                    18, 0, 0, 0,
                                    /* Value */
                                    /* Free physical memory pages (42) */
                                    42, 0, 0, 0,
                                    /* number of EBB state entries */
                                    1, 0, 0, 0,
                                    /* EBB state */
                                    0x01, 0, 0, 0,
                                    /* number of page alloc entries */
                                    1, 0, 0, 0,
                                    /* page alloc */
                                    42, 0x00,
                                    /* Tag for HPSRAM_STATE: 1 */
                                    1, 0, 0, 0,
                                    /* Length: (1+(1+2) + (1+1.5)) * 6.5 */
                                    26, 0, 0, 0,
                                    /* Value */
                                    /* Free physical memory pages (1337 = 0x0539) */
                                    0x39, 0x05, 0, 0,
                                    /* number of EBB state entries */
                                    2, 0, 0, 0,
                                    /* EBB state, part 1 */
                                    0xff, 0x00, 0x00, 0x00,
                                    /* EBB state, part 2 */
                                    0xff, 0xff, 0xff, 0xff,
                                    /* number of page alloc entries */
                                    3, 0, 0, 0,
                                    /* page alloc, parts 1, 2 & 3 */
                                    42, 0x00, 0x00, 0x00, 0x00, 0xff};

        /* Add command for get module parameter */
        commands.addGetGlobalMemoryStateCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                                globalMemoryState);
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

    /* start the perf service */
    CHECK_NOTHROW(client.request(
        "/instance/cavs.perf_measurement/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("perfservice_started")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    /* 1: Getting system information*/
    CHECK_NOTHROW(client.request(
        "/instance/cavs/0/info_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("subsystem_instance_info_params"))));
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
            AecParameterId, maxParameterPayloadSize, aecControlParameterPayload);
        commands.addGetModuleParameterCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, InstanceId,
            dsp_fw::ParameterId{25}, maxParameterPayloadSize, nsControlParameterPayload);
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

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: GET module instance info parameters "
                          "(URL: /instance/cavs.module-aec/1/info_parameters)",
                 "[module][info]")
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
        dsp_fw::ModuleInstanceProps props;
        props.id.moduleId = 1;
        props.id.instanceId = 1;
        props.stack_bytes = 128;
        props.bss_total_bytes = 256;
        props.bss_used_bytes = 64;
        commands.addGetModuleInstancePropsCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                                  InstanceId, props);
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
        "/instance/cavs.module-aec/1/info_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("module_instance_info_params"))));
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
        /* 0 is sent twice as far as we cannot wake another core than 0 separately. */
        commands.addSetCorePowerCommand(true, 0, false);

        /* 2: Set log parameter to
        * - isStarted : true
        * - level: verbose
        * - output: sram
        */
        linux::MockedControlDeviceCommands controlCommands(*controlDevice);
        /* 1: Get log parameter, will return
        * - isStarted : false  (Up to the logger that does not start yet the compress devices
        * - level: critical (Up to the control device that get the level using ctl mixer)
        * - output: Sram (not used)
        */
        controlCommands.addGetLogLevelCommand(true, linux::mixer_ctl::LogPriority::Critical);

        /* 2: Set log parameter to
        * - isStarted : true
        * - level: verbose
        * - output: sram
        */
        controlCommands.addSetLogLevelCommand(true, linux::mixer_ctl::LogPriority::Verbose);

        /* 3: Get log parameter , will return
        * - isStarted : true
        * - level: verbose
        * - output: sram
        */
        controlCommands.addGetLogLevelCommand(true, linux::mixer_ctl::LogPriority::Verbose);

        commands.addSetCorePowerCommand(true, 0, true);
        /* 0 is sent twice as far as we cannot wake another core than 0 separately. */
        commands.addSetCorePowerCommand(true, 0 /*1*/, true);
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
        /* 0 is sent twice as far as we cannot wake another core than 0 separately. */
        commands.addSetCorePowerCommand(true, 0 /*1*/, false);

        /** mock set log level commands. */
        linux::MockedControlDeviceCommands controlCommands(*controlDevice);
        controlCommands.addGetLogLevelCommand(true, linux::mixer_ctl::LogPriority::Critical);
        controlCommands.addSetLogLevelCommand(true, linux::mixer_ctl::LogPriority::Verbose);

        commands.addSetCorePowerCommand(true, 0, true);
        /* 0 is sent twice as far as we cannot wake another core than 0 separately. */
        commands.addSetCorePowerCommand(true, 0 /*1*/, true);
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
        HttpClientSimulator::Status::InternalError, "text/plain",
        HttpClientSimulator::StringContent(
            "Internal error: ParameterDispatcher: cannot set control parameter value: "
            "Unable to set log parameters: Unable to set log parameter: Can not change log "
            "parameters while logging is activated. (type=cavs.fwlogs kind=Control instance=0\n"
            "value:\n"
            "<control_parameters>\n"
            "    <BooleanParameter Name=\"Started\">1</BooleanParameter>\n"
            "    <ParameterBlock Name=\"Buffering\">\n"
            "        <IntegerParameter Name=\"Size\">100</IntegerParameter>\n"
            "        <BooleanParameter Name=\"Circular\">0</BooleanParameter>\n"
            "    </ParameterBlock>\n"
            "    <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>\n"
            "    <EnumParameter Name=\"Verbosity\">Verbose</EnumParameter>\n"
            "    <BooleanParameter Name=\"ViaPTI\">0</BooleanParameter>\n"
            "</control_parameters>\n)")));
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: performance measurement service", "[perf]")
{
    // This test case only covers the perf service's lifecycle. For data retrieval, see the
    // Subsystem info parameters test case.

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
}

// Probing constants
static const std::size_t probeCount = 8;
static const std::size_t injectionProbeIndex = 0;
static const std::size_t extractionProbeIndex = 1;
static_assert(extractionProbeIndex < probeCount, "wrong extraction probe index");
static_assert(injectionProbeIndex < probeCount, "wrong injection probe index");

static const dsp_fw::BitDepth bitDepth = dsp_fw::BitDepth::DEPTH_16BIT;
static const std::size_t channelCount = 4;
static const std::size_t sampleByteSize = (dsp_fw::BitDepth::DEPTH_16BIT / 8) * channelCount;
static const std::size_t injectionProbeDevicePeriodSize = 31;

// To test corner cases better
static_assert(injectionProbeDevicePeriodSize % sampleByteSize != 0,
              "Ring buffer size shall not "
              "be aligned with sample byte size");

/**
 * Generate a buffer filled with 20 probe extraction packets. Packet size is deduced from
 * the index [0..20[
 */
Buffer generateProbeExtractionContent(dsp_fw::ProbePointId probePointId)
{
    MemoryByteStreamWriter writer;
    for (uint32_t i = 0; i < 20; i++) {
        dsp_fw::Packet packet;
        packet.format = 0;
        packet.dspWallClockTsHw = 0;
        packet.dspWallClockTsLw = 0;
        packet.probePointId = probePointId;

        // using index as size and byte value
        packet.data.resize(i, i);
        writer.write(packet);
    }
    return writer.getBuffer();
}

/**
 * Split a buffer into chunks.
 * Each chunk size is deduced from a size list, using this formula:
 * chunk_size[i] = sizeList[ i % sizeList.size() ]
 */
std::vector<Buffer> splitBuffer(const util::Buffer &buffer, std::vector<std::size_t> sizeList)
{
    std::size_t current = 0;
    std::size_t i = 0;
    std::vector<Buffer> buffers;
    while (current < buffer.size()) {
        std::size_t chunkSize = std::min(sizeList[i % sizeList.size()], buffer.size() - current);
        buffers.emplace_back(buffer.begin() + current, buffer.begin() + current + chunkSize);
        current += chunkSize;
        ++i;
    }
    return buffers;
}

/** Create data for probe injection */
Buffer createInjectionData()
{
    static const std::size_t sampleCount = 100;
    static std::size_t byteCount = sampleCount * sampleByteSize;
    Buffer buffer(byteCount);
    for (std::size_t i = 0; i < byteCount; i++) {
        buffer[i] = i % 256;
    }
    return buffer;
}

/** Create injection expected ring buffer content and consumer position list from injected data.
 *
 * @param[in] data injected data
 * @return the list of expected ring buffers
 */
std::vector<Buffer> createExpectedInjectionBuffers(const util::Buffer &data)
{
    /** Consumer position will be incremented by this value */
    static const std::size_t consumerPositionDelta = injectionProbeDevicePeriodSize;
    static_assert(consumerPositionDelta % sampleByteSize != 0,
                  "consumerPositionDelta shall "
                  "not be a multiple of sampleByteSize");

    static const std::size_t ringBufferSampleCount =
        injectionProbeDevicePeriodSize / sampleByteSize;

    std::vector<Buffer> buffers;
    std::size_t consumerPosition = 0;
    MemoryInputStream is(data);

    // Prefilling buffer with silence
    util::Buffer block(ringBufferSampleCount * sampleByteSize, 0);
    buffers.push_back(block);

    // Then filling buffer from injected data
    while (!is.isEOS()) {

        // calculating next block size
        std::size_t availableBytes = injectionProbeDevicePeriodSize;
        std::size_t availableSamples = availableBytes / sampleByteSize;
        std::size_t availableSampleBytes = availableSamples * sampleByteSize;
        block.resize(availableSampleBytes);

        // Reading injected data
        auto read = is.read(block.data(), block.size());
        if (read < block.size()) {

            // Completing with silence if needed
            std::fill(block.begin() + read, block.end(), 0);
        }

        // Saving buffer
        buffers.push_back(block);
    };

    return buffers;
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: probe service control nominal cases", "[probe]")
{
    /* Setting the test vector
     * ----------------------- */
    // Generating probe extraction content
    const dsp_fw::ProbePointId probePointId(1, 2, dsp_fw::ProbeType::Output, 0);
    Buffer extractedContent = generateProbeExtractionContent(probePointId);

    // Splitting extraction content in blocks : each block will be written to the mocked compress
    // device
    // Block sizes are {1, 10, 20, 30} (cycling)
    auto blocks = splitBuffer(extractedContent, {1, 10, 20, 30});

    // Creating the mocked compress probe extractor device,
    // with a size that matches the max block size, in order
    // to test the case (written data size == ring buffer size)

    // Creating injection data
    Buffer injectData = createInjectionData();

    // Creating expected injection blocks
    auto expectedInjectionBlocks = createExpectedInjectionBuffers(injectData);

    std::unique_ptr<linux::MockedCompressDeviceFactory> compressDeviceFactory =
        std::make_unique<linux::MockedCompressDeviceFactory>();
    compressDevice->addSuccessfulCompressDeviceEntryOpen();
    compressDevice->addSuccessfulCompressDeviceEntryStart();
    for (auto &block : blocks) {
        compressDevice->addSuccessfulCompressDeviceEntryWait(0, true);
        compressDevice->addSuccessfulCompressDeviceEntryRead(block, block.size());
    }
    // At the end, the stream will be stopped, the compress wait shall return an error, so
    // and exception is raised. It will be translated into an error.
    compressDevice->addFailedCompressDeviceEntryWait(linux::CompressDevice::mInfiniteTimeout, true);
    compressDevice->addSuccessfulCompressDeviceEntryStop();
    // Use the mocked compress device of the fixture as the prober extraction device.
    compressDeviceFactory->setMockedProbeExtractDevice(std::move(compressDevice));

    // Injection probe device mock configuration
    compressProbeInjectDevice->addSuccessfulCompressDeviceEntryOpen();
    // First avail shall return the size of the compress device ring buffer (since empty)
    std::shared_ptr<SyncWait> syncWaiter(new SyncWait);

    for (auto &expectedBlock : expectedInjectionBlocks) {
        if (&expectedBlock == &expectedInjectionBlocks.front()) {
            compressProbeInjectDevice->addSuccessfulCompressDeviceEntryGetBufferSize(
                expectedBlock.size());
            compressProbeInjectDevice->addSuccessfulCompressDeviceEntryWrite(expectedBlock,
                                                                             expectedBlock.size());
            // The first write will start the device as per design
            compressProbeInjectDevice->addSuccessfulCompressDeviceEntryStart();
            // Only for the first one we shall expect a sync for the sake of test and buffer match
            compressProbeInjectDevice->addSuccessfulCompressDeviceEntryWait(
                linux::CompressDevice::mInfiniteTimeout, true, syncWaiter);
            continue;
        }
        compressProbeInjectDevice->addSuccessfulCompressDeviceEntryAvail(expectedBlock.size());
        compressProbeInjectDevice->addSuccessfulCompressDeviceEntryWrite(expectedBlock,
                                                                         expectedBlock.size());
        if (&expectedBlock != &expectedInjectionBlocks.back()) {
            compressProbeInjectDevice->addSuccessfulCompressDeviceEntryWait(0, true);
        }
    }
    // At the end, the stream will be stopped, the compress wait shall return an error, so
    // and exception is raised. It will be translated into an error.
    compressProbeInjectDevice->addFailedCompressDeviceEntryWait(
        linux::CompressDevice::mInfiniteTimeout, true);
    compressProbeInjectDevice->addSuccessfulCompressDeviceEntryStop();
    // Use the mocked compress device of the fixture as the prober extraction device.
    compressDeviceFactory->addMockedProbeInjectDevice(std::move(compressProbeInjectDevice));

    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        using Type = dsp_fw::ProbeType;
        using Purpose = Prober::ProbePurpose;
        // setting probe configuration (probe #1 is enabled)
        cavs::Prober::SessionProbes probes = {{true, {1, 2, Type::Input, 0}, Purpose::Inject},
                                              {true, {1, 2, Type::Output, 0}, Purpose::Extract},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject}};

        linux::MockedControlDeviceCommands controlCommands(*controlDevice);

        // For step 1 : Getting probe service parameters, checking that it is stopped

        // For step 3 : Configuring probe #1 to be enabled in extraction
        // 3 : Configuring probe #0 to be enabled for injection and probe #1 to be enabled for
        //     extraction
        std::size_t probeIndex = 0;
        for (auto &probe : probes) {
            linux::mixer_ctl::ProbeControl probeControl(linux::Prober::toLinux(probe));
            controlCommands.addSetProbeControlCommand(true, probeIndex, probeControl);
            ++probeIndex;
        }

        // For step 4 : Getting probe endpoint parameters, checking that they are deactivated except
        // the one that has been enabled

        // For step 5: enabling probe service with injection probe enabled involve to retrieve
        // a module instance props in order to calculate sample byte size
        dsp_fw::PinProps pinProps;
        pinProps.stream_type = static_cast<dsp_fw::StreamType>(0);
        pinProps.phys_queue_id = 0;
        pinProps.format.valid_bit_depth = 0;
        pinProps.format.bit_depth = bitDepth;
        pinProps.format.number_of_channels = channelCount;

        dsp_fw::ModuleInstanceProps moduleInstanceProps;
        moduleInstanceProps.id.moduleId = 0;
        moduleInstanceProps.id.instanceId = 0;
        moduleInstanceProps.input_pins.pin_info.push_back(pinProps);

        commands.addGetModuleInstancePropsCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, 1, 2,
                                                  moduleInstanceProps);
    }

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(std::move(device), std::move(controlDevice),
                                                      std::move(compressDeviceFactory));

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

    // 3 : Configuring probe #0 to be enabled for injection and probe #1 to be enabled for
    //     extraction
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe.endpoint/" + std::to_string(extractionProbeIndex) +
            "/control_parameters",
        HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_endpoint_param_enabled_extract")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe.endpoint/" + std::to_string(injectionProbeIndex) +
            "/control_parameters",
        HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_endpoint_param_enabled_inject")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    // 4 : Getting probe endpoint parameters, checking that they are deactivated except the one that
    //     has been enabled
    for (std::size_t probeIndex = 0; probeIndex < probeCount; ++probeIndex) {
        std::string expectedFile;
        if (probeIndex == extractionProbeIndex) {
            expectedFile = "probeservice_endpoint_param_enabled_extract";
        } else if (probeIndex == injectionProbeIndex) {
            expectedFile = "probeservice_endpoint_param_enabled_inject";
        } else {
            expectedFile = "probeservice_endpoint_param_disabled";
        }

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

    // 7 : Extract from an enabled probe.
    auto future = std::async(std::launch::async, [&] {
        client.request("/instance/cavs.probe.endpoint/" + std::to_string(extractionProbeIndex) +
                           "/streaming",
                       HttpClientSimulator::Verb::Get, "", HttpClientSimulator::Status::Ok,
                       "application/vnd.ifdk-file",
                       HttpClientSimulator::FileContent(dataPath + "probe_1_extraction.bin"));
    });

    // Sending inject data to the DBGA
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe.endpoint/" + std::to_string(injectionProbeIndex) + "/streaming",
        HttpClientSimulator::Verb::Put, injectData, HttpClientSimulator::Status::Ok, "",
        HttpClientSimulator::StringContent("")));

    // Now the stream injection has been done, trig the probe inject device to consume data.
    syncWaiter->unblockWait();

    // 8 : Stopping service
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_param_stopped")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    // Checking that step 7 has not failed
    CHECK_NOTHROW(future.get());

    // 9: Getting probe service parameters, checking that it is stopped
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("probeservice_param_stopped"))));
}

TEST_CASE_METHOD(Fixture, "DebugAgent/cAVS: probe service control failure cases", "[probe]")
{
    static const std::size_t probeCount = 8;
    static const std::size_t extractionProbeIndex = 1;

    /* Setting the test vector
    * ----------------------- */
    std::unique_ptr<linux::MockedCompressDeviceFactory> compressDeviceFactory =
        std::make_unique<linux::MockedCompressDeviceFactory>();
    compressDevice->addFailedCompressDeviceEntryOpen();
    // Use the mocked compress device of the fixture as the prober extraction device.
    compressDeviceFactory->setMockedProbeExtractDevice(std::move(compressDevice));

    {
        linux::MockedDeviceCommands commands(*device);
        DBGACommandScope scope(commands);

        using Type = dsp_fw::ProbeType;
        using Purpose = Prober::ProbePurpose;
        const dsp_fw::ProbePointId probePointId(1, 2, dsp_fw::ProbeType::Output, 0);
        cavs::Prober::SessionProbes probes = {{false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {true, probePointId, Purpose::Extract}, // Enabled
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject},
                                              {false, {0, 0, Type::Input, 0}, Purpose::Inject}};

        linux::MockedControlDeviceCommands controlCommands(*controlDevice);

        // For step 1 : Getting probe service parameters, checking that it is stopped

        // For step 3 : Configuring probe #1 to be enabled
        std::size_t probeIndex = 0;
        for (auto &probe : probes) {
            linux::mixer_ctl::ProbeControl probeControl(linux::Prober::toLinux(probe));
            controlCommands.addSetProbeControlCommand(true, probeIndex, probeControl);
            ++probeIndex;
        }

        // For step 4 : Getting probe endpoint parameters, checking that they are deactivated
        //     except the one that has been enabled
    }

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the factory that will inject the mocked device */
    linux::DeviceInjectionDriverFactory driverFactory(std::move(device), std::move(controlDevice),
                                                      std::move(compressDeviceFactory));

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
        "/instance/cavs.probe.endpoint/" + std::to_string(extractionProbeIndex) +
            "/control_parameters",
        HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_endpoint_param_enabled_extract")),
        HttpClientSimulator::Status::Ok, "", HttpClientSimulator::StringContent("")));

    // 4 : Getting probe endpoint parameters, checking that they are deactivated except the one that
    //     has been enabled
    for (std::size_t probeIndex = 0; probeIndex < probeCount; ++probeIndex) {
        std::string expectedFile = (probeIndex == extractionProbeIndex)
                                       ? "probeservice_endpoint_param_enabled_extract"
                                       : "probeservice_endpoint_param_disabled";

        CHECK_NOTHROW(client.request(
            "/instance/cavs.probe.endpoint/" + std::to_string(probeIndex) + "/control_parameters",
            HttpClientSimulator::Verb::Get, "", HttpClientSimulator::Status::Ok, "text/xml",
            HttpClientSimulator::FileContent(xmlFileName(expectedFile))));
    }

    // 5 : If service starting fails, it should come back to "Idle" state
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Put,
        file_helper::readAsString(xmlFileName("probeservice_param_started")),
        HttpClientSimulator::Status::InternalError, "text/plain",
        HttpClientSimulator::StringContent(
            "Internal error: ParameterDispatcher: cannot set "
            "control parameter value: Unable to set probe service state: Cannot set probe service "
            "state: Could not start extraction input stream: Error opening Extraction Device: "
            "error during compress open: error#MockDevice "
            "(type=cavs.probe kind=Control instance=0\nvalue:\n"
            "<control_parameters>\n"
            "    <ParameterBlock Name=\"State\">\n"
            "        <BooleanParameter Name=\"Started\">1</BooleanParameter>\n"
            "    </ParameterBlock>\n"
            "</control_parameters>\n\n)")));

    // 3 : getting state: should be Idle
    CHECK_NOTHROW(client.request(
        "/instance/cavs.probe/0/control_parameters", HttpClientSimulator::Verb::Get, "",
        HttpClientSimulator::Status::Ok, "text/xml",
        HttpClientSimulator::FileContent(xmlFileName("probeservice_param_stopped"))));
}
