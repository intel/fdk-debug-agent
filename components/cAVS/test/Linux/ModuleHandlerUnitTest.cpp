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

#include "TestCommon/TestHelpers.hpp"
#include "cAVS/Linux/MockedDevice.hpp"
#include "cAVS/Linux/MockedDeviceCommands.hpp"
#include "cAVS/Linux/MockedDeviceCatchHelper.hpp"
#include "cAVS/Linux/ModuleHandler.hpp"
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

TEST_CASE_METHOD(Fixture, "Module handling: getting FW configs")
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
                                 0x05, 0x06, 0x07, 0x08};

    const uint8_t *buf = fwConfigTlvList.data();

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(*device);

    /* Successful get fw config command */
    commands.addGetFwConfigCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, fwConfigTlvList);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    dsp_fw::FwConfig fwConfig;

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

TEST_CASE_METHOD(Fixture, "Module handling: getting module parameter")
{
    static const Buffer fwParameterPayload = {1, 2, 3};

    static const uint16_t moduleId = 1;
    static const uint16_t instanceId = 2;
    static const dsp_fw::ParameterId parameterId(2);

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(*device);

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /* Successful command */
    commands.addGetModuleParameterCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
                                          parameterId, fwParameterPayload);

    /* Now using the mocked device
    * --------------------------- */

    Buffer parameterPayload(fwParameterPayload.size());

    /*Successful command */
    CHECK_NOTHROW(moduleHandler.getModuleParameter(moduleId, instanceId,
                                                   dsp_fw::ParameterId(parameterId),
                                                   parameterPayload, parameterPayload.size()));
    CHECK(fwParameterPayload == parameterPayload);
}

TEST_CASE_METHOD(Fixture, "Module handling: set module enable")
{
    const uint16_t moduleId = 1;
    const uint16_t instanceId = 2;
    const dsp_fw::ParameterId parameterId{dsp_fw::BaseModuleParams::MOD_INST_ENABLE};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(*device);

    /* Successful command */
    commands.addSetModuleParameterCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
                                          parameterId);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /*Successful command */
    CHECK_NOTHROW(moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, {}));
}

TEST_CASE_METHOD(Fixture, "Module handling: setting module parameter")
{
    const Buffer parameterPayload = {4, 5, 6};

    const uint16_t moduleId = 1;
    const uint16_t instanceId = 2;
    const dsp_fw::ParameterId parameterId{2};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(*device);

    /* Successful command */
    commands.addSetModuleParameterCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
                                          parameterId, parameterPayload);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /*Successful command */
    CHECK_NOTHROW(
        moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, parameterPayload));
}

TEST_CASE_METHOD(Fixture, "Module handling: setting loadable module parameter")
{
    const Buffer parameterPayload = {4, 5, 6};

    const uint16_t moduleId = 0x1024;
    const uint16_t instanceId = 2;
    const dsp_fw::ParameterId parameterId{2};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(*device);

    /* Successful command */
    commands.addSetModuleParameterCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId, instanceId,
                                          parameterId, parameterPayload);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /*Successful command */
    CHECK_NOTHROW(
        moduleHandler.setModuleParameter(moduleId, instanceId, parameterId, parameterPayload));
}

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
void checkModuleEntry(linux::ModuleHandler &moduleHandler, std::size_t expectedModuleCount,
                      const std::vector<dsp_fw::ModuleEntry> &expectedModuleEntry)
{
    /*Successful get module info command */
    std::vector<dsp_fw::ModuleEntry> entries;
    CHECK_NOTHROW(
        moduleHandler.getModulesEntries(static_cast<uint32_t>(expectedModuleCount), entries));

    /* Checking result */

    /* First check expected module count. */
    CHECK(entries.size() == expectedModuleCount);

    /* Then check expected Module entry matches the returned Module Entries. */
    CHECK(entries == expectedModuleEntry);
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

TEST_CASE_METHOD(Fixture, "Module handling: getting module entries")
{
    const uint32_t moduleCount = 2;

    /* Setting the test vector
     * ----------------------- */
    MockedDeviceCommands commands(*device);

    std::vector<dsp_fw::ModuleEntry> expectedModuleEntry = produceModuleEntries(moduleCount);

    /* Successful get module info command with 2 modules */
    commands.addGetModuleEntriesCommand(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleCount,
                                        expectedModuleEntry);

    /* Now using the mocked device
     * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /*Successful get module info command with 2 modules*/
    checkModuleEntry(moduleHandler, moduleCount, expectedModuleEntry);
}

TEST_CASE_METHOD(Fixture, "Module handling: getting pipeline list")
{
    static const uint32_t fwMaxPplCount = 10;
    using ID = dsp_fw::PipeLineIdType;
    static const std::vector<ID> fwPipelineIdList = {ID{1}, ID{2}, ID{3}};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(*device);

    /* Successful get pipeline list command */
    commands.addGetPipelineListCommand(
        /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, fwMaxPplCount,
        fwPipelineIdList);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /* Simulating an os error during getting pipeline list */
    std::vector<dsp_fw::PipeLineIdType> pipelineIds;
    static const uint32_t maxPipeline = 10;

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
    MockedDeviceCommands commands(*device);

    /* Successful command */
    commands.addGetPipelinePropsCommand(
        /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, pipelineId, fwProps);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /* Simulating an os error */

    static const dsp_fw::PplProps emptyProps = {PlID{0}, 0, 0, 0, 0, 0, {}, {}, {}};
    dsp_fw::PplProps props = emptyProps;

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
    MockedDeviceCommands commands(*device);

    /* Successful command */
    commands.addGetSchedulersInfoCommand(
        /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, coreId, fwSchedulersInfo);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /* Simulating an os error */

    static const dsp_fw::SchedulersInfo emptyInfo = {};
    dsp_fw::SchedulersInfo info = emptyInfo;

    /*Successful command */
    CHECK_NOTHROW(moduleHandler.getSchedulersInfo(coreId, info));
    CHECK(fwSchedulersInfo == info);
}

bool isSameGateway(const dsp_fw::GatewayProps &a, const dsp_fw::GatewayProps &b)
{
    return a.attribs == b.attribs && a.id == b.id;
}

TEST_CASE_METHOD(Fixture, "Module handling: getting gateways")
{
    static const uint32_t fwGatewayCount = 10;
    static const std::vector<dsp_fw::GatewayProps> fwGateways = {{1, 2}, {3, 4}};

    /* Setting the test vector
    * ----------------------- */
    MockedDeviceCommands commands(*device);

    /* Successful command */
    commands.addGetGatewaysCommand(/*true, STATUS_SUCCESS,*/ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
                                   fwGatewayCount, fwGateways);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

    /* Simulating an os error during getting pipeline list */
    std::vector<dsp_fw::GatewayProps> gateways;

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
    MockedDeviceCommands commands(*device);

    /* Successful command */
    commands.addGetModuleInstancePropsCommand(/*true, STATUS_SUCCESS,*/
                                              dsp_fw::IxcStatus::ADSP_IPC_SUCCESS, moduleId,
                                              instanceId, fwInstanceProps);

    /* Now using the mocked device
    * --------------------------- */

    /* Creating the module handler, that will use the mocked device*/
    linux::ModuleHandler moduleHandler(*device);

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

    /*Successful command */
    CHECK_NOTHROW(moduleHandler.getModuleInstanceProps(moduleId, instanceId, props));
    CHECK(fwInstanceProps == props);
}
