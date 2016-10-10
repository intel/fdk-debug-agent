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

#include "cAVS/Linux/MockedDeviceCommands.hpp"
#include "cAVS/ModuleHandler.hpp"
#include "cAVS/Linux/DriverTypes.hpp"
#include "Util/Buffer.hpp"

#define NB_EL(x) (sizeof(x) / sizeof((x)[0]))

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

void MockedDeviceCommands::addTlvParameterCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                                  const Buffer &tlvList,
                                                  dsp_fw::BaseFwParams parameterId)
{
    addGetModuleParameterCommand(returnedFirmwareStatus, dsp_fw::baseFirmwareModuleId,
                                 dsp_fw::baseFirmwareInstanceId, dsp_fw::toParameterId(parameterId),
                                 tlvBufferSize, tlvList);
}

void MockedDeviceCommands::addGetHwConfigCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                                 const Buffer &hwConfigTlvList)
{
    addTlvParameterCommand(returnedFirmwareStatus, hwConfigTlvList,
                           dsp_fw::BaseFwParams::HW_CONFIG_GET);
}

void MockedDeviceCommands::addGetFwConfigCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                                 const Buffer &fwConfigTlvList)
{
    addTlvParameterCommand(returnedFirmwareStatus, fwConfigTlvList,
                           dsp_fw::BaseFwParams::FW_CONFIG);
}

void MockedDeviceCommands::addGetPipelineListCommand(
    dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t maxPplCount,
    const std::vector<dsp_fw::PipeLineIdType> &pipelineIds)
{
    std::size_t parameterSize = dsp_fw::PipelinesListInfo::getAllocationSize(maxPplCount);

    dsp_fw::PipelinesListInfo pipelinesListInfo;
    pipelinesListInfo.ppl_id = pipelineIds;

    util::MemoryByteStreamWriter writer;
    writer.write(pipelinesListInfo);

    util::Buffer returnedOutput(parameterSize, 0xFF);
    returnedOutput = writer.getBuffer();

    addGetModuleParameterCommand(returnedFirmwareStatus, dsp_fw::baseFirmwareModuleId,
                                 dsp_fw::baseFirmwareInstanceId,
                                 dsp_fw::ParameterId(dsp_fw::BaseFwParams::PIPELINE_LIST_INFO_GET),
                                 parameterSize, returnedOutput);
}

void MockedDeviceCommands::addGetPipelinePropsCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                                      dsp_fw::PipeLineIdType pipelineId,
                                                      const dsp_fw::PplProps &props)
{
    /* Using extended parameter id to supply the pipeline id*/
    auto paramId =
        ModuleHandler::getExtendedParameterId(dsp_fw::BaseFwParams::PIPELINE_PROPS_GET, pipelineId);

    util::MemoryByteStreamWriter writer;
    writer.write(props);

    util::Buffer returnedOutput(maxParameterPayloadSize, 0xFF);
    returnedOutput = writer.getBuffer();

    addGetModuleParameterCommand(returnedFirmwareStatus, dsp_fw::baseFirmwareModuleId,
                                 dsp_fw::baseFirmwareInstanceId, paramId, maxParameterPayloadSize,
                                 returnedOutput);
}

void MockedDeviceCommands::addGetSchedulersInfoCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                                       dsp_fw::CoreId coreId,
                                                       const dsp_fw::SchedulersInfo &info)
{
    /* Using extended parameter id to supply the core id*/
    auto paramId =
        ModuleHandler::getExtendedParameterId(dsp_fw::BaseFwParams::SCHEDULERS_INFO_GET, coreId);

    util::MemoryByteStreamWriter writer;
    writer.write(info);

    util::Buffer returnedOutput(maxParameterPayloadSize, 0xFF);
    returnedOutput = writer.getBuffer();

    addGetModuleParameterCommand(returnedFirmwareStatus, dsp_fw::baseFirmwareModuleId,
                                 dsp_fw::baseFirmwareInstanceId, paramId, maxParameterPayloadSize,
                                 returnedOutput);
}

void MockedDeviceCommands::addGetGatewaysCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                                 uint32_t gatewayCount,
                                                 const std::vector<dsp_fw::GatewayProps> &gateways)
{
    /* Calculating the memory space required */
    std::size_t parameterSize = dsp_fw::GatewaysInfo::getAllocationSize(gatewayCount);

    dsp_fw::GatewaysInfo gatewaysInfo;
    gatewaysInfo.gateways = gateways;

    util::MemoryByteStreamWriter writer;
    writer.write(gatewaysInfo);

    util::Buffer returnedOutput(parameterSize, 0xFF);
    returnedOutput = writer.getBuffer();

    addGetModuleParameterCommand(returnedFirmwareStatus, dsp_fw::baseFirmwareModuleId,
                                 dsp_fw::baseFirmwareInstanceId,
                                 dsp_fw::toParameterId(dsp_fw::BaseFwParams::GATEWAYS_INFO_GET),
                                 parameterSize, returnedOutput);
}

void MockedDeviceCommands::addGetModuleInstancePropsCommand(
    dsp_fw::IxcStatus returnedFirmwareStatus, uint16_t moduleId, uint16_t instanceId,
    const dsp_fw::ModuleInstanceProps &props)
{
    util::MemoryByteStreamWriter writer;
    writer.write(props);

    util::Buffer returnedOutput(maxParameterPayloadSize, 0xFF);
    returnedOutput = writer.getBuffer();

    addGetModuleParameterCommand(returnedFirmwareStatus, moduleId, instanceId,
                                 dsp_fw::toParameterId(dsp_fw::BaseModuleParams::MOD_INST_PROPS),
                                 maxParameterPayloadSize, returnedOutput);
}

void MockedDeviceCommands::addGetModuleEntriesCommand(
    dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t moduleCount,
    const std::vector<dsp_fw::ModuleEntry> &returnedEntries)
{
    std::size_t moduleInfoSize = dsp_fw::ModulesInfo::getAllocationSize(moduleCount);

    dsp_fw::ModulesInfo modulesInfo;
    modulesInfo.module_info = returnedEntries;

    util::MemoryByteStreamWriter writer;
    writer.write(modulesInfo);

    util::Buffer returnedOutput(moduleInfoSize, 0xFF);
    returnedOutput = writer.getBuffer();

    addGetModuleParameterCommand(returnedFirmwareStatus, dsp_fw::baseFirmwareModuleId,
                                 dsp_fw::baseFirmwareInstanceId,
                                 dsp_fw::ParameterId(dsp_fw::BaseFwParams::MODULES_INFO_GET),
                                 moduleInfoSize, returnedOutput);
}

void MockedDeviceCommands::addGetGlobalPerfDataCommand(
    dsp_fw::IxcStatus returnedFirmwareStatus, uint32_t maxItemCount,
    const std::vector<dsp_fw::PerfDataItem> &perfItems)
{
    /* Calculating the memory space required */
    std::size_t parameterSize = dsp_fw::GlobalPerfData::getAllocationSize(maxItemCount);

    dsp_fw::GlobalPerfData perfData;
    perfData.items = perfItems;

    util::MemoryByteStreamWriter writer;
    writer.write(perfData);

    util::Buffer returnedOutput(parameterSize, 0xFF);
    returnedOutput = writer.getBuffer();

    addGetModuleParameterCommand(returnedFirmwareStatus, dsp_fw::baseFirmwareModuleId,
                                 dsp_fw::baseFirmwareInstanceId,
                                 dsp_fw::toParameterId(dsp_fw::BaseFwParams::GLOBAL_PERF_DATA),
                                 parameterSize, returnedOutput);
}

void MockedDeviceCommands::addGetGlobalMemoryStateCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                                          const Buffer &memoryTlvList)
{
    addTlvParameterCommand(returnedFirmwareStatus, memoryTlvList,
                           dsp_fw::BaseFwParams::MEMORY_STATE_INFO_GET);
}

void MockedDeviceCommands::addGetModuleParameterCommand(dsp_fw::IxcStatus returnedFirmwareStatus,
                                                        uint16_t moduleId, uint16_t instanceId,
                                                        dsp_fw::ParameterId parameterId,
                                                        const util::Buffer &parameterPayload)
{
    addGetModuleParameterCommand(returnedFirmwareStatus, moduleId, instanceId, parameterId,
                                 parameterPayload.size(), parameterPayload);
}

void MockedDeviceCommands::addSetModuleParameterCommand(dsp_fw::IxcStatus, uint16_t moduleId,
                                                        uint16_t instanceId,
                                                        dsp_fw::ParameterId parameterId,
                                                        const util::Buffer &parameterPayload)
{
    /* Fill the test vector for get parameter command and reply */
    driver::LargeConfigAccess largeConfigAccess(driver::LargeConfigAccess::CmdType::Set, moduleId,
                                                instanceId, parameterId.getValue(),
                                                parameterPayload.size(), parameterPayload);

    /* write parameter access structure to file to send the getParameter command*/
    util::MemoryByteStreamWriter messageWriter;
    messageWriter.write(largeConfigAccess);
    util::Buffer sentMessage = messageWriter.getBuffer();

    addSetCorePowerCommand(true, 0, false);
    mDevice.addCommandWriteOK(driver::setGetCtrl, sentMessage, sentMessage.size());
    addSetCorePowerCommand(true, 0, true);
}

void MockedDeviceCommands::addSetModuleParameterCommand(dsp_fw::IxcStatus, uint16_t moduleId,
                                                        uint16_t instanceId,
                                                        dsp_fw::ParameterId parameterId)
{
    /* Fill the test vector for get parameter command and reply */
    driver::ModuleConfigAccess configAccess(driver::ModuleConfigAccess::CmdType::Set, moduleId,
                                            instanceId, parameterId.getValue());

    /* write parameter access structure to file to send the getParameter command*/
    util::MemoryByteStreamWriter messageWriter;
    messageWriter.write(configAccess);
    util::Buffer sentMessage = messageWriter.getBuffer();

    addSetCorePowerCommand(true, 0, false);
    mDevice.addCommandWriteOK(driver::setGetCtrl, sentMessage, sentMessage.size());
    addSetCorePowerCommand(true, 0, true);
}

void MockedDeviceCommands::addGetModuleParameterCommand(dsp_fw::IxcStatus, uint16_t moduleId,
                                                        uint16_t instanceId,
                                                        dsp_fw::ParameterId parameterId,
                                                        size_t parameterSize,
                                                        const util::Buffer &parameterPayload)
{
    /* Fill the test vector for get parameter command and reply */
    driver::LargeConfigAccess largeConfigAccess(driver::LargeConfigAccess::CmdType::Get, moduleId,
                                                instanceId, parameterId.getValue(), parameterSize);

    /* write parameter access structure to file to send the getParameter command*/
    util::MemoryByteStreamWriter messageWriter;
    messageWriter.write(largeConfigAccess);

    /* Format the expected buffer to be read from the written command, i.e.
     * the parameter access structure appended with the parameter payload. */
    util::MemoryByteStreamWriter returnedReadMsgWriter = {};
    if (driver::requireTunneledAccess(moduleId, parameterId.getValue())) {
        driver::TunneledHeader tunneledAccess = {parameterId.getValue(),
                                                 (uint32_t)parameterPayload.size()};
        returnedReadMsgWriter.write(tunneledAccess);
    };
    returnedReadMsgWriter.writeRawBuffer(parameterPayload);

    /** The read command output buffer is expected to be sized at param payload max size. */
    util::Buffer outputBuffer;
    outputBuffer.resize(maxParameterPayloadSize);

    addSetCorePowerCommand(true, 0, false);
    mDevice.addCommandReadOK(driver::setGetCtrl, messageWriter.getBuffer(), outputBuffer,
                             returnedReadMsgWriter.getBuffer());
    addSetCorePowerCommand(true, 0, true);
}

void MockedDeviceCommands::addSetCorePowerCommand(bool controlSuccess, unsigned int coreId,
                                                  bool allowedToSleep)
{
    driver::CorePowerCommand corePowerCmd(allowedToSleep, coreId);
    if (controlSuccess) {
        mDevice.addCommandWriteOK(driver::corePowerCtrl, corePowerCmd.getBuffer(),
                                  corePowerCmd.getBuffer().size());
    } else {
        mDevice.addCommandWriteKO(driver::corePowerCtrl, corePowerCmd.getBuffer(),
                                  corePowerCmd.getBuffer().size());
    }
}

void MockedDeviceCommands::addSetPerfState(Perf::State state)
{
    util::MemoryByteStreamWriter toDriver;
    toDriver.write(static_cast<uint32_t>(state));
    addSetModuleParameterCommand(
        /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
        dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
        dsp_fw::ParameterId{dsp_fw::BaseFwParams::PERF_MEASUREMENTS_STATE}, toDriver.getBuffer());
}

void MockedDeviceCommands::addGetPerfState(Perf::State state)
{
    util::MemoryByteStreamWriter toDriver;
    toDriver.write(static_cast<uint32_t>(state));
    addGetModuleParameterCommand(
        /*true, STATUS_SUCCESS, */ dsp_fw::IxcStatus::ADSP_IPC_SUCCESS,
        dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
        dsp_fw::ParameterId{dsp_fw::BaseFwParams::PERF_MEASUREMENTS_STATE}, toDriver.getBuffer());
}
}
}
}
