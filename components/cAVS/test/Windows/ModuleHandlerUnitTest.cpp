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
#include "cAVS/Windows/MockedDeviceCatchHelper.hpp"
#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "cAVS/Windows/ModuleHandler.hpp"
#include "Util/Buffer.hpp"
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
using namespace debug_agent::util;

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
    uint8_t *buf = reinterpret_cast<uint8_t *>(&value);
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

bool isSameGateway(const dsp_fw::GatewayProps &a, const dsp_fw::GatewayProps &b)
{
    return a.attribs == b.attribs && a.id == b.id;
}

/** Perform a module entry ioctl and check the result using the supplied expected module count */
void checkModuleEntryIoctl(windows::ModuleHandler &moduleHandler, std::size_t expectedModuleCount)
{
    /*Successful get module info command */
    std::vector<dsp_fw::ModuleEntry> entries;
    CHECK_NOTHROW(
        moduleHandler.getModulesEntries(static_cast<uint32_t>(expectedModuleCount), entries));

    /* Checking result */
    dsp_fw::ModuleEntry expectedModuleEntry;
    setArbitraryContent(expectedModuleEntry);
    CHECK(entries.size() == expectedModuleCount);
    for (auto &candidateModuleEntry : entries) {
        CHECK(memoryEquals(candidateModuleEntry, expectedModuleEntry));
    }
}

using Fixture = MockedDeviceFixture;

TEST_CASE_METHOD(Fixture, "Module handling: getting module entries")
{
    const static uint32_t moduleCount = 2;

    /* Setting the test vector
     * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error during getting module entries */
    commands.addGetModuleEntriesCommand(false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                        moduleCount,
                                        std::vector<dsp_fw::ModuleEntry>()); /* unused parameter */

    /* Simulating a driver error during getting module entries */
    commands.addGetModuleEntriesCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                        dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleCount,
                                        std::vector<dsp_fw::ModuleEntry>()); /* unused parameter */

    /* Simulating a firmware error during getting module entries */
    commands.addGetModuleEntriesCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE,
                                        moduleCount,
                                        std::vector<dsp_fw::ModuleEntry>()); /* unused parameter */

    /* Successful get module info command with 2 modules */
    commands.addGetModuleEntriesCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                        moduleCount, produceModuleEntries(moduleCount));

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error during getting module entries */
    std::vector<dsp_fw::ModuleEntry> entries;
    CHECK_THROWS_AS_MSG(moduleHandler.getModulesEntries(moduleCount, entries),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Device returns an exception: OS says that io control has failed.");
    CHECK(entries.empty());

    /* Simulating a driver error during getting module entries */
    CHECK_THROWS_AS_MSG(moduleHandler.getModulesEntries(moduleCount, entries),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(entries.empty());

    /* Simulating a firmware error during getting module entries */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getModulesEntries(moduleCount, entries),
        debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));

    CHECK(entries.empty());

    /*Successful get module info command with 2 modules*/
    checkModuleEntryIoctl(moduleHandler, 2);
}

TEST_CASE_METHOD(Fixture, "Module handling: getting FW configs")
{
    static const size_t fwVersionValueOffsetInTlv = 8;

    static const Buffer fwConfigTlvList{/* Tag for FW_VERSION: 0x00000000 */
                                        0x00, 0x00, 0x00, 0x00,
                                        /* Length = 8 bytes */
                                        0x08, 0x00, 0x00, 0x00,
                                        /* Value: dsp_fw::FwVersion */
                                        /* major and minor */
                                        0x01, 0x02, 0x03, 0x04,
                                        /* hot fix and build */
                                        0x05, 0x06, 0x07, 0x08};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error during getting fw config */
    commands.addGetFwConfigCommand(false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   fwConfigTlvList); /* unused parameter */

    /* Simulating a driver error during getting fw config */
    commands.addGetFwConfigCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                   dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   fwConfigTlvList); /* unused parameter */

    /* Simulating a firmware error during getting fw config */
    commands.addGetFwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE,
                                   fwConfigTlvList); /* unused parameter */

    /* Successful get fw config command */
    commands.addGetFwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   fwConfigTlvList);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error during getting fw config */
    dsp_fw::FwConfig fwConfig;
    CHECK_THROWS_AS_MSG(moduleHandler.getFwConfig(fwConfig),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Device returns an exception: OS says that io control has failed.");
    CHECK(fwConfig.isFwVersionValid == false);

    /* Simulating a driver error during getting fw config */
    CHECK_THROWS_AS_MSG(moduleHandler.getFwConfig(fwConfig),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(fwConfig.isFwVersionValid == false);

    /* Simulating a firmware error during getting fw config */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getFwConfig(fwConfig), debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
    CHECK(fwConfig.isFwVersionValid == false);

    /* Successful get fw config command */
    CHECK_NOTHROW(moduleHandler.getFwConfig(fwConfig));
    CHECK(fwConfig.isFwVersionValid == true);
    const dsp_fw::FwVersion *injectedVersion = reinterpret_cast<const dsp_fw::FwVersion *>(
        fwConfigTlvList.data() + fwVersionValueOffsetInTlv);
    // No operator== in FW type: compare each field individually:
    CHECK(fwConfig.fwVersion.major == injectedVersion->major);
    CHECK(fwConfig.fwVersion.minor == injectedVersion->minor);
    CHECK(fwConfig.fwVersion.hotfix == injectedVersion->hotfix);
    CHECK(fwConfig.fwVersion.build == injectedVersion->build);
}

TEST_CASE_METHOD(Fixture, "Module handling: getting pipeline list")
{
    static const uint32_t fwMaxPplCount = 10;
    using ID = dsp_fw::PipeLineIdType;
    static const std::vector<ID> fwPipelineIdList = {ID{1}, ID{2}, ID{3}};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error during getting pipeline list */
    commands.addGetPipelineListCommand(false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                       fwMaxPplCount,
                                       std::vector<dsp_fw::PipeLineIdType>()); // unused parameter

    /* Simulating a driver error during getting pipeline list  */
    commands.addGetPipelineListCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                       dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, fwMaxPplCount,
                                       std::vector<dsp_fw::PipeLineIdType>()); // unused parameter

    /* Simulating a firmware error during getting pipeline list  */
    commands.addGetPipelineListCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE,
                                       fwMaxPplCount,
                                       std::vector<dsp_fw::PipeLineIdType>()); // unused parameter

    /* Successful get pipeline list command */
    commands.addGetPipelineListCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                       fwMaxPplCount, fwPipelineIdList);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error during getting pipeline list */
    std::vector<dsp_fw::PipeLineIdType> pipelineIds;
    static const uint32_t maxPipeline = 10;
    CHECK_THROWS_AS_MSG(moduleHandler.getPipelineIdList(maxPipeline, pipelineIds),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Device returns an exception: OS says that io control has failed.");
    CHECK(pipelineIds.empty());

    /* Simulating a driver error during getting pipeline list */
    CHECK_THROWS_AS_MSG(moduleHandler.getPipelineIdList(maxPipeline, pipelineIds),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(pipelineIds.empty());

    /* Simulating a firmware error during getting pipeline list */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getPipelineIdList(maxPipeline, pipelineIds),
        debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
    CHECK(pipelineIds.empty());

    /*Successful get pipeline list command */
    CHECK_NOTHROW(moduleHandler.getPipelineIdList(maxPipeline, pipelineIds));
    CHECK(fwPipelineIdList == pipelineIds);
}

TEST_CASE_METHOD(Fixture, "Module handling: getting pipeline props")
{
    using PlID = dsp_fw::PipeLineIdType;
    static const PlID pipelineId{1};
    static const dsp_fw::PplProps fwProps = {PlID{1}, 2, 3, 4, 5, 6, {{1, 0}, {2, 0}, {3, 0}},
                                             {4, 5},  {}};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error */
    commands.addGetPipelinePropsCommand(false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                        pipelineId, dsp_fw::PplProps()); /* unused parameter */

    /* Simulating a driver error */
    commands.addGetPipelinePropsCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                        dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, pipelineId,
                                        dsp_fw::PplProps()); /* unused parameter */

    /* Simulating a firmware error */
    commands.addGetPipelinePropsCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE,
                                        pipelineId, dsp_fw::PplProps()); /* unused parameter */

    /* Successful command */
    commands.addGetPipelinePropsCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                        pipelineId, fwProps);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error */

    static const dsp_fw::PplProps emptyProps = {PlID{0}, 0, 0, 0, 0, 0, {}, {}, {}};
    dsp_fw::PplProps props = emptyProps;

    CHECK_THROWS_AS_MSG(moduleHandler.getPipelineProps(pipelineId, props),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Device returns an exception: OS says that io control has failed.");
    CHECK(emptyProps == props);

    /* Simulating a driver error */
    CHECK_THROWS_AS_MSG(moduleHandler.getPipelineProps(pipelineId, props),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(emptyProps == props);

    /* Simulating a firmware error */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getPipelineProps(pipelineId, props),
        debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
    CHECK(emptyProps == props);

    /*Successful command */
    CHECK_NOTHROW(moduleHandler.getPipelineProps(pipelineId, props));
    CHECK(props == fwProps);
}

TEST_CASE_METHOD(Fixture, "Module handling: getting schedulers info")
{
    static const dsp_fw::CoreId coreId{1};

    static const dsp_fw::TaskProps task1 = {3, {{1, 0}, {2, 0}}};
    static const dsp_fw::TaskProps task2 = {4, {{8, 0}}};
    static const dsp_fw::TaskProps task3 = {6, {}};

    static const dsp_fw::SchedulerProps props1 = {1, 2, {task1, task2}};
    static const dsp_fw::SchedulerProps props2 = {4, 2, {task3}};

    static const dsp_fw::SchedulersInfo fwSchedulersInfo = {{props1, props2}};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error */
    commands.addGetSchedulersInfoCommand(false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                         coreId, dsp_fw::SchedulersInfo()); /* unused parameter */

    /* Simulating a driver error */
    commands.addGetSchedulersInfoCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                         dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, coreId,
                                         dsp_fw::SchedulersInfo()); /* unused parameter */

    /* Simulating a firmware error */
    commands.addGetSchedulersInfoCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE,
                                         coreId, dsp_fw::SchedulersInfo()); /* unused parameter */

    /* Successful command */
    commands.addGetSchedulersInfoCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                         coreId, fwSchedulersInfo);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error */

    static const dsp_fw::SchedulersInfo emptyInfo = {};
    dsp_fw::SchedulersInfo info = emptyInfo;

    CHECK_THROWS_AS_MSG(moduleHandler.getSchedulersInfo(coreId, info),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Device returns an exception: OS says that io control has failed.");
    CHECK(emptyInfo == info);

    /* Simulating a driver error */
    CHECK_THROWS_AS_MSG(moduleHandler.getSchedulersInfo(coreId, info),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(emptyInfo == info);

    /* Simulating a firmware error */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getSchedulersInfo(coreId, info), debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
    CHECK(emptyInfo == info);

    /*Successful command */
    CHECK_NOTHROW(moduleHandler.getSchedulersInfo(coreId, info));
    CHECK(fwSchedulersInfo == info);
}

TEST_CASE_METHOD(Fixture, "Module handling: getting gateways")
{
    static const uint32_t fwGatewayCount = 10;
    static const std::vector<dsp_fw::GatewayProps> fwGateways = {{1, 2}, {3, 4}};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error */
    commands.addGetGatewaysCommand(false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   fwGatewayCount,
                                   std::vector<dsp_fw::GatewayProps>()); /* unused parameter */

    /* Simulating a driver error during */
    commands.addGetGatewaysCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                   dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, fwGatewayCount,
                                   std::vector<dsp_fw::GatewayProps>()); /* unused parameter */

    /* Simulating a firmware error during */
    commands.addGetGatewaysCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE,
                                   fwGatewayCount,
                                   std::vector<dsp_fw::GatewayProps>()); /* unused parameter */

    /* Successful command */
    commands.addGetGatewaysCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   fwGatewayCount, fwGateways);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error during getting pipeline list */
    std::vector<dsp_fw::GatewayProps> gateways;
    CHECK_THROWS_AS_MSG(moduleHandler.getGatewaysInfo(fwGatewayCount, gateways),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Device returns an exception: OS says that io control has failed.");
    CHECK(gateways.empty());

    /* Simulating a driver error during getting pipeline list */
    CHECK_THROWS_AS_MSG(moduleHandler.getGatewaysInfo(fwGatewayCount, gateways),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(gateways.empty());

    /* Simulating a firmware error during getting pipeline list */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getGatewaysInfo(fwGatewayCount, gateways),
        debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
    CHECK(gateways.empty());

    /*Successful get pipeline list command */
    CHECK_NOTHROW(moduleHandler.getGatewaysInfo(fwGatewayCount, gateways));
    CHECK(fwGateways.size() == gateways.size());
    CHECK(std::equal(fwGateways.begin(), fwGateways.end(), gateways.begin(), isSameGateway));
}

TEST_CASE_METHOD(Fixture, "Module handling: getting module instance properties")
{
    static const dsp_fw::AudioDataFormatIpc audioFormat = {
        static_cast<dsp_fw::SamplingFrequency>(1),
        static_cast<dsp_fw::BitDepth>(2),
        static_cast<dsp_fw::ChannelMap>(3),
        static_cast<dsp_fw::ChannelConfig>(4),
        static_cast<dsp_fw::InterleavingStyle>(5),
        6,
        7,
        static_cast<dsp_fw::SampleType>(8),
        9};

    static const dsp_fw::PinListInfo input_pins = {
        {{static_cast<dsp_fw::StreamType>(1), audioFormat, 3}}};
    static const dsp_fw::PinListInfo output_pins = {
        {{static_cast<dsp_fw::StreamType>(4), audioFormat, 5},
         {static_cast<dsp_fw::StreamType>(6), audioFormat, 7}}};

    static const dsp_fw::ModuleInstanceProps fwInstanceProps = {{1, 0},
                                                                2,
                                                                3,
                                                                4,
                                                                5,
                                                                6,
                                                                7,
                                                                8,
                                                                9,
                                                                10,
                                                                11,
                                                                input_pins,
                                                                output_pins,
                                                                dsp_fw::ConnectorNodeId(12),
                                                                dsp_fw::ConnectorNodeId(13)};

    static const uint16_t moduleId = 1;
    static const uint16_t instanceId = 2;

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error */
    commands.addGetModuleInstancePropsCommand(
        false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
        dsp_fw::ModuleInstanceProps()); /* unused parameter */

    /* Simulating a driver error */
    commands.addGetModuleInstancePropsCommand(
        true, STATUS_FLOAT_DIVIDE_BY_ZERO, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
        instanceId, dsp_fw::ModuleInstanceProps()); /* unused parameter */

    /* Simulating a firmware error */
    commands.addGetModuleInstancePropsCommand(
        true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE, moduleId, instanceId,
        dsp_fw::ModuleInstanceProps()); /* unused parameter */

    /* Successful command */
    commands.addGetModuleInstancePropsCommand(true, STATUS_SUCCESS,
                                              dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, fwInstanceProps);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error */

    static const dsp_fw::ModuleInstanceProps emptyProps = {{0, 0},
                                                           0,
                                                           0,
                                                           0,
                                                           0,
                                                           0,
                                                           0,
                                                           0,
                                                           0,
                                                           0,
                                                           0,
                                                           dsp_fw::PinListInfo(),
                                                           dsp_fw::PinListInfo(),
                                                           dsp_fw::ConnectorNodeId(0),
                                                           dsp_fw::ConnectorNodeId(0)};
    dsp_fw::ModuleInstanceProps props = emptyProps;

    CHECK_THROWS_AS_MSG(moduleHandler.getModuleInstanceProps(moduleId, instanceId, props),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Device returns an exception: OS says that io control has failed.");
    CHECK(emptyProps == props);

    /* Simulating a driver error */
    CHECK_THROWS_AS_MSG(moduleHandler.getModuleInstanceProps(moduleId, instanceId, props),
                        debug_agent::cavs::ModuleHandler::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(emptyProps == props);

    /* Simulating a firmware error */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getModuleInstanceProps(moduleId, instanceId, props),
        debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
    CHECK(emptyProps == props);

    /*Successful command */
    CHECK_NOTHROW(moduleHandler.getModuleInstanceProps(moduleId, instanceId, props));
    CHECK(fwInstanceProps == props);
}

TEST_CASE_METHOD(Fixture, "Module handling: getting module parameter")
{
    static const Buffer fwParameterPayload = {1, 2, 3};

    static const uint16_t moduleId = 1;
    static const uint16_t instanceId = 2;
    static const dsp_fw::ParameterId parameterId{2};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error */
    commands.addGetModuleParameterCommand(false, STATUS_SUCCESS,
                                          dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
                                          parameterId, Buffer()); /* unused parameter */

    /* Simulating a driver error */
    commands.addGetModuleParameterCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                          dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
                                          parameterId, Buffer()); /* unused parameter */

    /* Simulating a firmware error */
    commands.addGetModuleParameterCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE,
                                          moduleId, instanceId, parameterId,
                                          Buffer()); /* unused parameter */

    /* Successful command */
    commands.addGetModuleParameterCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                          moduleId, instanceId, parameterId, fwParameterPayload);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error */
    Buffer parameterPayload;

    CHECK_THROWS_AS_MSG(
        moduleHandler.getModuleParameter(moduleId, instanceId, parameterId, parameterPayload),
        debug_agent::cavs::ModuleHandler::Exception,
        "Device returns an exception: OS says that io control has failed.");
    CHECK(parameterPayload.empty());

    /* Simulating a driver error */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getModuleParameter(moduleId, instanceId, parameterId, parameterPayload),
        debug_agent::cavs::ModuleHandler::Exception,
        "Driver returns invalid status: " +
            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
    CHECK(parameterPayload.empty());

    /* Simulating a firmware error */
    CHECK_THROWS_AS_MSG(
        moduleHandler.getModuleParameter(moduleId, instanceId, parameterId, parameterPayload),
        debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
    CHECK(parameterPayload.empty());

    /*Successful command */
    CHECK_NOTHROW(
        moduleHandler.getModuleParameter(moduleId, instanceId, parameterId, parameterPayload));
    CHECK(fwParameterPayload == parameterPayload);
}

TEST_CASE_METHOD(Fixture, "Module handling: setting module parameter")
{
    static const Buffer parameterPayload = {4, 5, 6};

    static const uint16_t moduleId = 1;
    static const uint16_t instanceId = 2;
    static const dsp_fw::ParameterId parameterId{2};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    /* Simulating an os error */
    commands.addSetModuleParameterCommand(false, STATUS_SUCCESS,
                                          dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
                                          parameterId, parameterPayload);

    /* Simulating a driver error */
    commands.addSetModuleParameterCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                          dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
                                          parameterId, parameterPayload);

    /* Simulating a firmware error */
    commands.addSetModuleParameterCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE,
                                          moduleId, instanceId, parameterId, parameterPayload);

    /* Successful command */
    commands.addSetModuleParameterCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                          moduleId, instanceId, parameterId, parameterPayload);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    windows::ModuleHandler moduleHandler(device);

    /* Simulating an os error */
    CHECK_THROWS_AS_MSG(
        moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, parameterPayload),
        debug_agent::cavs::ModuleHandler::Exception,
        "Device returns an exception: OS says that io control has failed.");

    /* Simulating a driver error */
    CHECK_THROWS_AS_MSG(
        moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, parameterPayload),
        debug_agent::cavs::ModuleHandler::Exception,
        "Driver returns invalid status: " +
            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));

    /* Simulating a firmware error */
    CHECK_THROWS_AS_MSG(
        moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, parameterPayload),
        debug_agent::cavs::ModuleHandler::Exception,
        "Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));

    /*Successful command */
    CHECK_NOTHROW(
        moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, parameterPayload));
}
