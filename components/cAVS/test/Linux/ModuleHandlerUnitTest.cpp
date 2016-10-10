/*
 * Copyright (c) 2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TestCommon/TestHelpers.hpp"
#include "cAVS/Linux/MockedDevice.hpp"
#include "cAVS/Linux/MockedDeviceCommands.hpp"
#include "cAVS/Linux/MockedDeviceCatchHelper.hpp"
#include "cAVS/Linux/ModuleHandlerImpl.hpp"
#include "cAVS/ModuleHandler.hpp"
#include "Util/Buffer.hpp"
#include <catch.hpp>
#include <memory>
#include <iostream>
#include <string.h>

/**
 * NOTE: test vector buffers are filled with firmware and driver types, maybe a better way
 * would consist in using a bitstream in order to construct ioctl buffers from scratch.
 * To be discussed...
 */

using namespace debug_agent::cavs;
using namespace debug_agent::cavs::linux;
using namespace debug_agent::util;

using Fixture = MockedDeviceFixture;

static constexpr uint32_t moduleCount = 2;
/** Compares type memory content */
template <typename T>
bool operator==(const T &v1, const T &v2)
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

/** Perform a module entry ioctl and check the result using the supplied expected module count */
void checkModuleEntry(ModuleHandler &moduleHandler,
                      const std::vector<dsp_fw::ModuleEntry> &expectedModuleEntry)
{
    /*Successful get module info command */
    std::vector<dsp_fw::ModuleEntry> entries;
    CHECK_NOTHROW(entries = moduleHandler.getModuleEntries());

    /* Checking result */

    /* First check expected module count. */
    CHECK(entries.size() == moduleCount);

    /* Then check expected Module entry matches the returned Module Entries. */
    CHECK(entries == expectedModuleEntry);
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

TEST_CASE_METHOD(Fixture, "Module handler construction failure", "[module_handler]")
{
    MockedDeviceCommands commands(*device);

    // empty (aka erroneous) configs
    commands.addGetFwConfigCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, {});
    commands.addGetHwConfigCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, {});

    CHECK_THROWS_AS_MSG(ModuleHandler(std::make_unique<linux::ModuleHandlerImpl>(*device)),
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
    const size_t fwVersionValueOffsetInTlv = 8;

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
    MockedDeviceCommands commands(*device);

    commands.addGetFwConfigCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, fwConfigTlvList);
    commands.addGetHwConfigCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, hwConfigTlvList);
    commands.addGetModuleEntriesCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleCount,
                                        expectedModuleEntry);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    ModuleHandler moduleHandler(std::make_unique<linux::ModuleHandlerImpl>(*device));

    SECTION ("Getting FW configs") {
        auto &fwConfig = moduleHandler.getFwConfig();

        CHECK(fwConfig.isFwVersionValid == true);
        const dsp_fw::FwVersion *injectedVersion = reinterpret_cast<const dsp_fw::FwVersion *>(
            fwConfigTlvList.data() + fwVersionValueOffsetInTlv);
        // No operator== in FW type: compare each field individually:
        CHECK(fwConfig.fwVersion.major == injectedVersion->major);
        CHECK(fwConfig.fwVersion.minor == injectedVersion->minor);
        CHECK(fwConfig.fwVersion.hotfix == injectedVersion->hotfix);
        CHECK(fwConfig.fwVersion.build == injectedVersion->build);
    }
    SECTION ("Module handling: getting module parameter") {
        static const Buffer fwParameterPayload = {1, 2, 3};

        static const uint16_t moduleId = 1;
        static const uint16_t instanceId = 2;
        static const dsp_fw::ParameterId parameterId(2);

        Buffer parameterPayload(fwParameterPayload.size());

        /*Successful command */
        commands.addGetModuleParameterCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, parameterId, fwParameterPayload);
        CHECK_NOTHROW(
            parameterPayload = moduleHandler.getModuleParameter(
                moduleId, instanceId, dsp_fw::ParameterId(parameterId), parameterPayload.size()));
        CHECK(fwParameterPayload == parameterPayload);
    }

    SECTION ("Set module enable") {
        const uint16_t moduleId = 1;
        const uint16_t instanceId = 2;
        const dsp_fw::ParameterId parameterId{dsp_fw::BaseModuleParams::MOD_INST_ENABLE};

        /* Successful command */
        commands.addSetModuleParameterCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, parameterId);
        CHECK_NOTHROW(moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, {}));
    }

    SECTION ("Setting module parameter") {
        const Buffer parameterPayload = {4, 5, 6};

        const uint16_t moduleId = 1;
        const uint16_t instanceId = 2;
        const dsp_fw::ParameterId parameterId{2};

        /* Successful command */
        commands.addSetModuleParameterCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, parameterId, parameterPayload);
        CHECK_NOTHROW(
            moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, parameterPayload));
    }

    SECTION ("Setting loadable module parameter") {
        const Buffer parameterPayload = {4, 5, 6};

        const uint16_t moduleId = 0x1024;
        const uint16_t instanceId = 2;
        const dsp_fw::ParameterId parameterId{2};

        /* Successful command */
        commands.addSetModuleParameterCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, parameterId, parameterPayload);
        CHECK_NOTHROW(
            moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, parameterPayload));
    }

    SECTION ("Getting module entries") {
        /*Successful get module info command with 2 modules*/
        checkModuleEntry(moduleHandler, expectedModuleEntry);
    }

    SECTION ("Getting pipeline list") {
        using ID = dsp_fw::PipeLineIdType;
        static const std::vector<ID> fwPipelineIdList = {ID{1}, ID{2}, ID{3}};
        std::vector<dsp_fw::PipeLineIdType> pipelineIds;

        /* Successful get pipeline list command */
        commands.addGetPipelineListCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, fwPipelineIdList.size(),
            fwPipelineIdList);
        CHECK_NOTHROW(pipelineIds = moduleHandler.getPipelineIdList());
        CHECK(fwPipelineIdList == pipelineIds);
    }

    SECTION ("Getting pipeline props") {
        using PlID = dsp_fw::PipeLineIdType;
        static const PlID pipelineId{1};
        static const dsp_fw::PplProps fwProps = {PlID{1}, 2, 3, 4, 5, 6, {{1, 0}, {2, 0}, {3, 0}},
                                                 {4, 5},  {}};
        dsp_fw::PplProps props;

        /* Successful command */
        commands.addGetPipelinePropsCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, pipelineId, fwProps);
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

        dsp_fw::SchedulersInfo info;

        /* Successful command */
        commands.addGetSchedulersInfoCommand(
            /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, coreId,
            fwSchedulersInfo);
        CHECK_NOTHROW(info = moduleHandler.getSchedulersInfo(coreId));
        CHECK(fwSchedulersInfo == info);
    }

    SECTION ("Getting gateways") {
        static const std::vector<dsp_fw::GatewayProps> fwGateways = {{1, 2}, {3, 4}};

        /* Simulating an os error during getting pipeline list */
        std::vector<dsp_fw::GatewayProps> gateways;

        /* Successful command */
        commands.addGetGatewaysCommand(
            /*true, STATUS_SUCCESS,*/ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, fwGateways.size(),
            fwGateways);
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

        /* Successful command */
        commands.addGetModuleInstancePropsCommand(/*true, STATUS_SUCCESS,*/
                                                  dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                                  instanceId, fwInstanceProps);
        CHECK_NOTHROW(props = moduleHandler.getModuleInstanceProps(moduleId, instanceId));
        CHECK(fwInstanceProps == props);
    }

    SECTION ("Getting perf items") {
        static const std::vector<dsp_fw::PerfDataItem> expectedPerfItems = {
            dsp_fw::PerfDataItem(0, 0, false, false, 1337, 42),   // Core 0
            dsp_fw::PerfDataItem(1, 0, true, false, 123456, 789), // Module 1, instance 0
            dsp_fw::PerfDataItem(0, 1, true, true, 987654, 321),  // Core 1
            dsp_fw::PerfDataItem(12, 0, false, false, 1111, 222), // Module 12, instance 0
            dsp_fw::PerfDataItem(12, 1, true, false, 3333, 444)   // Module 12, instance 1
        };

        std::vector<dsp_fw::PerfDataItem> actualPerfItems;

        /* Successful command */
        commands.addGetGlobalPerfDataCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                             expectedPerfItems.size(), expectedPerfItems);
        CHECK_NOTHROW(actualPerfItems = moduleHandler.getPerfItems());
        CHECK(actualPerfItems == expectedPerfItems);
    }
}
