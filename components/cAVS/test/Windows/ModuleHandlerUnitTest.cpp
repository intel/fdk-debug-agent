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
#include "cAVS/ModuleHandler.hpp"
#include "cAVS/Windows/ModuleHandlerImpl.hpp"
#include "Util/Buffer.hpp"
#include <catch.hpp>
#include <memory>
#include <iostream>
#include <utility>

/**
 * NOTE: test vector buffers are filled with firmware and driver types, maybe a better way
 * would consist in using a bitstream in order to construct ioctl buffers from scratch.
 * To be discussed...
 */

using namespace debug_agent::cavs;
using namespace debug_agent::cavs::windows;
using namespace debug_agent::util;

static constexpr uint32_t moduleCount = 2;

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
std::vector<dsp_fw::ModuleEntry> produceModuleEntries()
{
    dsp_fw::ModuleEntry moduleEntry;
    setArbitraryContent(moduleEntry);

    std::vector<dsp_fw::ModuleEntry> entries;
    for (std::size_t i = 0; i < moduleCount; ++i) {
        entries.push_back(moduleEntry);
    }

    return entries;
}

/** Perform a module entry ioctl and check the result using the supplied expected module count */
void checkModuleEntryIoctl(ModuleHandler &moduleHandler)
{
    /*Successful get module info command */
    std::vector<dsp_fw::ModuleEntry> entries;
    CHECK_NOTHROW(entries = moduleHandler.getModuleEntries());

    /* Checking result */
    dsp_fw::ModuleEntry expectedModuleEntry;
    setArbitraryContent(expectedModuleEntry);
    CHECK(entries.size() == moduleCount);
    for (auto &candidateModuleEntry : entries) {
        CHECK(memoryEquals(candidateModuleEntry, expectedModuleEntry));
    }
}

using Fixture = MockedDeviceFixture;

TEST_CASE_METHOD(Fixture, "Module handler construction failure", "[module_handler]")
{
    MockedDeviceCommands commands(device);

    commands.addGetFwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, {});
    commands.addGetHwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, {});

    CHECK_THROWS_AS_MSG(ModuleHandler(std::make_unique<windows::ModuleHandlerImpl>(device)),
                        ModuleHandler::Exception,
                        "The following config items could not be retrieved from hardware:\n"
                        "FW version\n"
                        "Module count\n"
                        "Max pipeline count\n"
                        "Max module instance count\n"
                        "Gateway count\n"
                        "DSP core count\n");
}

TEST_CASE_METHOD(Fixture, "Module handler", "[module_handler]")
{
    const Buffer fwConfigTlvList{/* Tag for FW_VERSION: 0x00000000 */
                                 0x00, 0x00, 0x00, 0x00,
                                 /* Length = 8 bytes */
                                 0x08, 0x00, 0x00, 0x00,
                                 /* Value: dsp_fw::FwVersion */
                                 /* major and minor */
                                 0x01, 0x02, 0x03, 0x04,
                                 /* hot fix and build */
                                 0x05, 0x06, 0x07, 0x08,
                                 /* Tag for MAX_PPL_CNT_FW_CFG: 9 */
                                 9, 0x00, 0x00, 0x00,
                                 /* Len: 4 bytes */
                                 0x04, 0x00, 0x00, 0x00,
                                 /* value: 3*/
                                 0x03, 0x00, 0x00, 0x00,
                                 /* Tag for MODULES_COUNT_FW_CFG: 12*/
                                 12, 0x00, 0x00, 0x00,
                                 /* Len: 4 bytes */
                                 0x04, 0x00, 0x00, 0x00,
                                 /* value */
                                 uint8_t{moduleCount}, 0x00, 0x00, 0x00,
                                 /* Tag for MAX_MOD_INST_COUNT_FW_CFG: 13, */
                                 13, 0x00, 0x00, 0x00,
                                 /* Len: 4 bytes */
                                 0x04, 0x00, 0x00, 0x00,
                                 /* value */
                                 3, 0x00, 0x00, 0x00};

    const Buffer hwConfigTlvList{/* GATEWAY_COUNT_HW_CFG: 6 */
                                 0x06, 0x00, 0x00, 0x00,
                                 /* Length = 4 bytes */
                                 0x04, 0x00, 0x00, 0x00,
                                 /* Value: 2 */
                                 0x02, 0x00, 0x00, 0x00,
                                 /* Tag for DSP_CORES_HW_CFG: 1 */
                                 0x01, 0x00, 0x00, 0x00,
                                 /* Len: 4 bytes */
                                 0x04, 0x00, 0x00, 0x00,
                                 /* Value: 2 */
                                 2, 0x00, 0x00, 0x00};

    std::vector<dsp_fw::ModuleEntry> expectedModuleEntry = produceModuleEntries();

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(device);

    commands.addGetFwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   fwConfigTlvList);
    commands.addGetHwConfigCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   hwConfigTlvList);
    commands.addGetModuleEntriesCommand(true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                        moduleCount, expectedModuleEntry);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    ModuleHandler moduleHandler(std::make_unique<windows::ModuleHandlerImpl>(device));

    SECTION ("Getting FW configs") {
        auto &fwConfig = moduleHandler.getFwConfig();
        static const size_t fwVersionValueOffsetInTlv = 8;

        CHECK(fwConfig.isFwVersionValid == true);
        const dsp_fw::FwVersion *injectedVersion = reinterpret_cast<const dsp_fw::FwVersion *>(
            fwConfigTlvList.data() + fwVersionValueOffsetInTlv);
        // No operator== in FW type: compare each field individually:
        CHECK(fwConfig.fwVersion.major == injectedVersion->major);
        CHECK(fwConfig.fwVersion.minor == injectedVersion->minor);
        CHECK(fwConfig.fwVersion.hotfix == injectedVersion->hotfix);
        CHECK(fwConfig.fwVersion.build == injectedVersion->build);
    }

    SECTION ("Getting module parameter") {
        static const Buffer fwParameterPayload = {1, 2, 3};

        static const uint16_t moduleId = 1;
        static const uint16_t instanceId = 2;
        static const dsp_fw::ParameterId parameterId{2};

        /* Simulating an os error */
        commands.addGetModuleParameterCommand(
            false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
            parameterId, Buffer()); /* unused parameter */

        /* Simulating a driver error */
        commands.addGetModuleParameterCommand(
            true, STATUS_FLOAT_DIVIDE_BY_ZERO, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
            instanceId, parameterId, Buffer()); /* unused parameter */

        /* Simulating a firmware error */
        commands.addGetModuleParameterCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE, moduleId, instanceId,
            parameterId, Buffer()); /* unused parameter */

        /* Successful command */
        commands.addGetModuleParameterCommand(true, STATUS_SUCCESS,
                                              dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, parameterId, fwParameterPayload);

        /* Simulating an os error */
        Buffer parameterPayload;

        CHECK_THROWS_AS_MSG(parameterPayload =
                                moduleHandler.getModuleParameter(moduleId, instanceId, parameterId),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Device returns an exception: OS says that io control has failed.");
        CHECK(parameterPayload.empty());

        /* Simulating a driver error */
        CHECK_THROWS_AS_MSG(parameterPayload =
                                moduleHandler.getModuleParameter(moduleId, instanceId, parameterId),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Driver returns invalid status: " +
                                std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
        CHECK(parameterPayload.empty());

        /* Simulating a firmware error */
        CHECK_THROWS_AS_MSG(
            parameterPayload = moduleHandler.getModuleParameter(moduleId, instanceId, parameterId),
            debug_agent::cavs::ModuleHandler::Exception,
            "Firmware returns invalid status: " +
                std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
        CHECK(parameterPayload.empty());

        /*Successful command */
        CHECK_NOTHROW(parameterPayload =
                          moduleHandler.getModuleParameter(moduleId, instanceId, parameterId));
        CHECK(fwParameterPayload == parameterPayload);
    }

    SECTION ("Setting module parameter") {
        static const Buffer parameterPayload = {4, 5, 6};

        static const uint16_t moduleId = 1;
        static const uint16_t instanceId = 2;
        static const dsp_fw::ParameterId parameterId{2};

        /* Simulating an os error */
        commands.addSetModuleParameterCommand(false, STATUS_SUCCESS,
                                              dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, parameterId, parameterPayload);

        /* Simulating a driver error */
        commands.addSetModuleParameterCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                              dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, parameterId, parameterPayload);

        /* Simulating a firmware error */
        commands.addSetModuleParameterCommand(true, STATUS_SUCCESS,
                                              dsp_fw::IxcStatus::ADSP_IPC_FAILURE, moduleId,
                                              instanceId, parameterId, parameterPayload);

        /* Successful command */
        commands.addSetModuleParameterCommand(true, STATUS_SUCCESS,
                                              dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, parameterId, parameterPayload);

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

    SECTION ("Getting module entries") {
        /*Successful get module info command*/
        checkModuleEntryIoctl(moduleHandler);
    }

    SECTION ("Getting pipeline list") {
        static const uint32_t fwMaxPplCount = 9;
        using ID = dsp_fw::PipeLineIdType;
        static const std::vector<ID> fwPipelineIdList = {ID{1}, ID{2}, ID{3}};

        /* Simulating an os error during getting pipeline list */
        commands.addGetPipelineListCommand(
            false, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, 3,
            std::vector<dsp_fw::PipeLineIdType>()); // unused parameter

        /* Simulating a driver error during getting pipeline list  */
        commands.addGetPipelineListCommand(
            true, STATUS_FLOAT_DIVIDE_BY_ZERO, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, 3,
            std::vector<dsp_fw::PipeLineIdType>()); // unused parameter

        /* Simulating a firmware error during getting pipeline list  */
        commands.addGetPipelineListCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_FAILURE, 3,
            std::vector<dsp_fw::PipeLineIdType>()); // unused parameter

        /* Successful get pipeline list command */
        commands.addGetPipelineListCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, 3, fwPipelineIdList);

        /* Simulating an os error during getting pipeline list */
        std::vector<dsp_fw::PipeLineIdType> pipelineIds;
        CHECK_THROWS_AS_MSG(pipelineIds = moduleHandler.getPipelineIdList(),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Device returns an exception: OS says that io control has failed.");
        CHECK(pipelineIds.empty());

        /* Simulating a driver error during getting pipeline list */
        CHECK_THROWS_AS_MSG(pipelineIds = moduleHandler.getPipelineIdList(),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Driver returns invalid status: " +
                                std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
        CHECK(pipelineIds.empty());

        /* Simulating a firmware error during getting pipeline list */
        CHECK_THROWS_AS_MSG(
            pipelineIds = moduleHandler.getPipelineIdList(),
            debug_agent::cavs::ModuleHandler::Exception,
            "Firmware returns invalid status: " +
                std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
        CHECK(pipelineIds.empty());

        /*Successful get pipeline list command */
        CHECK_NOTHROW(pipelineIds = moduleHandler.getPipelineIdList());
        CHECK(fwPipelineIdList == pipelineIds);
    }

    SECTION ("Getting pipeline props") {
        using PlID = dsp_fw::PipeLineIdType;
        static const PlID pipelineId{1};
        static const dsp_fw::PplProps fwProps = {PlID{1}, 2, 3, 4, 5, 6, {{1, 0}, {2, 0}, {3, 0}},
                                                 {4, 5},  {}};

        /* Simulating an os error */
        commands.addGetPipelinePropsCommand(false, STATUS_SUCCESS,
                                            dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, pipelineId,
                                            dsp_fw::PplProps()); /* unused parameter */

        /* Simulating a driver error */
        commands.addGetPipelinePropsCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                            dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, pipelineId,
                                            dsp_fw::PplProps()); /* unused parameter */

        /* Simulating a firmware error */
        commands.addGetPipelinePropsCommand(true, STATUS_SUCCESS,
                                            dsp_fw::IxcStatus::ADSP_IPC_FAILURE, pipelineId,
                                            dsp_fw::PplProps()); /* unused parameter */

        /* Successful command */
        commands.addGetPipelinePropsCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, pipelineId, fwProps);

        /* Simulating an os error */

        static const dsp_fw::PplProps emptyProps = {PlID{0}, 0, 0, 0, 0, 0, {}, {}, {}};
        dsp_fw::PplProps props = emptyProps;

        CHECK_THROWS_AS_MSG(props = moduleHandler.getPipelineProps(pipelineId),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Device returns an exception: OS says that io control has failed.");
        CHECK(emptyProps == props);

        /* Simulating a driver error */
        CHECK_THROWS_AS_MSG(props = moduleHandler.getPipelineProps(pipelineId),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Driver returns invalid status: " +
                                std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
        CHECK(emptyProps == props);

        /* Simulating a firmware error */
        CHECK_THROWS_AS_MSG(
            props = moduleHandler.getPipelineProps(pipelineId),
            debug_agent::cavs::ModuleHandler::Exception,
            "Firmware returns invalid status: " +
                std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
        CHECK(emptyProps == props);

        /*Successful command */
        CHECK_NOTHROW(props = moduleHandler.getPipelineProps(pipelineId));
        CHECK(props == fwProps);
    }

    SECTION ("Getting schedulers info") {
        static const dsp_fw::CoreId coreId{1};

        static const dsp_fw::TaskProps task1 = {3, {{1, 0}, {2, 0}}};
        static const dsp_fw::TaskProps task2 = {4, {{8, 0}}};
        static const dsp_fw::TaskProps task3 = {6, {}};

        static const dsp_fw::SchedulerProps props1 = {1, 2, {task1, task2}};
        static const dsp_fw::SchedulerProps props2 = {4, 2, {task3}};

        static const dsp_fw::SchedulersInfo fwSchedulersInfo = {{props1, props2}};

        /* Simulating an os error */
        commands.addGetSchedulersInfoCommand(false, STATUS_SUCCESS,
                                             dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, coreId,
                                             dsp_fw::SchedulersInfo()); /* unused parameter */

        /* Simulating a driver error */
        commands.addGetSchedulersInfoCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                             dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, coreId,
                                             dsp_fw::SchedulersInfo()); /* unused parameter */

        /* Simulating a firmware error */
        commands.addGetSchedulersInfoCommand(true, STATUS_SUCCESS,
                                             dsp_fw::IxcStatus::ADSP_IPC_FAILURE, coreId,
                                             dsp_fw::SchedulersInfo()); /* unused parameter */

        /* Successful command */
        commands.addGetSchedulersInfoCommand(
            true, STATUS_SUCCESS, dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, coreId, fwSchedulersInfo);

        /* Simulating an os error */

        static const dsp_fw::SchedulersInfo emptyInfo = {};
        dsp_fw::SchedulersInfo info = emptyInfo;

        CHECK_THROWS_AS_MSG(info = moduleHandler.getSchedulersInfo(coreId),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Device returns an exception: OS says that io control has failed.");
        CHECK(emptyInfo == info);

        /* Simulating a driver error */
        CHECK_THROWS_AS_MSG(info = moduleHandler.getSchedulersInfo(coreId),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Driver returns invalid status: " +
                                std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
        CHECK(emptyInfo == info);

        /* Simulating a firmware error */
        CHECK_THROWS_AS_MSG(
            info = moduleHandler.getSchedulersInfo(coreId),
            debug_agent::cavs::ModuleHandler::Exception,
            "Firmware returns invalid status: " +
                std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
        CHECK(emptyInfo == info);

        /*Successful command */
        CHECK_NOTHROW(info = moduleHandler.getSchedulersInfo(coreId));
        CHECK(fwSchedulersInfo == info);
    }

    SECTION ("Getting gateways") {
        static const uint32_t fwGatewayCount = 2;
        static const std::vector<dsp_fw::GatewayProps> fwGateways = {{1, 2}, {3, 4}};

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

        /* Simulating an os error during getting pipeline list */
        std::vector<dsp_fw::GatewayProps> gateways;
        CHECK_THROWS_AS_MSG(gateways = moduleHandler.getGatewaysInfo(),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Device returns an exception: OS says that io control has failed.");
        CHECK(gateways.empty());

        /* Simulating a driver error during getting pipeline list */
        CHECK_THROWS_AS_MSG(gateways = moduleHandler.getGatewaysInfo(),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Driver returns invalid status: " +
                                std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
        CHECK(gateways.empty());

        /* Simulating a firmware error during getting pipeline list */
        CHECK_THROWS_AS_MSG(
            gateways = moduleHandler.getGatewaysInfo(), debug_agent::cavs::ModuleHandler::Exception,
            "Firmware returns invalid status: " +
                std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
        CHECK(gateways.empty());

        /*Successful get pipeline list command */
        CHECK_NOTHROW(gateways = moduleHandler.getGatewaysInfo());
        CHECK(fwGateways == gateways);
    }

    SECTION ("Getting module instance properties") {
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

        CHECK_THROWS_AS_MSG(props = moduleHandler.getModuleInstanceProps(moduleId, instanceId),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Device returns an exception: OS says that io control has failed.");
        CHECK(emptyProps == props);

        /* Simulating a driver error */
        CHECK_THROWS_AS_MSG(props = moduleHandler.getModuleInstanceProps(moduleId, instanceId),
                            debug_agent::cavs::ModuleHandler::Exception,
                            "Driver returns invalid status: " +
                                std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));
        CHECK(emptyProps == props);

        /* Simulating a firmware error */
        CHECK_THROWS_AS_MSG(
            props = moduleHandler.getModuleInstanceProps(moduleId, instanceId),
            debug_agent::cavs::ModuleHandler::Exception,
            "Firmware returns invalid status: " +
                std::to_string(static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_FAILURE)));
        CHECK(emptyProps == props);

        /*Successful command */
        CHECK_NOTHROW(props = moduleHandler.getModuleInstanceProps(moduleId, instanceId));
        CHECK(fwInstanceProps == props);
    }
}
