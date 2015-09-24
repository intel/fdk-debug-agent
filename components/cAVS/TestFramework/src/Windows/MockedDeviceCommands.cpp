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

#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "cAVS/Windows/ModuleHandler.hpp"
#include "Util/Buffer.hpp"

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace windows
{

void MockedDeviceCommands::addModuleParameterCommand(
    Command command,
    uint16_t moduleId,
    uint16_t instanceId,
    uint32_t parameterTypeId,
    const util::Buffer &expectedParameterContent,
    const util::Buffer &returnedParameterContent,
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus)
{
    /* Expected output buffer*/
    util::ByteStreamWriter expectedOutputWriter;

    /* Adding driver Intc_App_Cmd_Body structure */
    driver::Intc_App_Cmd_Body bodyCmd;
    bodyCmd.toStream(expectedOutputWriter);

    /* Adding driver IoctlFwModuleParam structure */
    driver::IoctlFwModuleParam moduleParam(moduleId, instanceId, parameterTypeId,
        static_cast<uint32_t>(expectedParameterContent.size()));
    moduleParam.toStream(expectedOutputWriter);

    /* Adding parameter content */
    expectedOutputWriter.writeRawBuffer(expectedParameterContent);

    /* Creating the ioctl input buffer*/
    util::ByteStreamWriter inputWriter;

    /* Adding driver Intc_App_Cmd_Header structure */
    driver::Intc_App_Cmd_Header ioctlInput(
        static_cast<uint32_t>(driver::IOCTL_FEATURE::FEATURE_FW_MODULE_PARAM),
        driver::moduleParameterAccessCommandParameterId,
        static_cast<uint32_t>(expectedOutputWriter.getBuffer().size()));
    ioctlInput.toStream(inputWriter);

    uint32_t ioctlCode = command == Command::Get ?
        IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET : IOCTL_CMD_APP_TO_AUDIODSP_BIG_SET;

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(
            ioctlCode,
            &inputWriter.getBuffer(),
            &expectedOutputWriter.getBuffer());
        return;
    }

    /* Expected returned buffer*/
    util::ByteStreamWriter returnedOutputWriter;

    /* Adding driver Intc_App_Cmd_Body structure */
    bodyCmd.Status = returnedDriverStatus;
    bodyCmd.toStream(returnedOutputWriter);

    if (NT_SUCCESS(returnedDriverStatus)) {

        /* Adding driver IoctlFwModuleParam structure */
        moduleParam.fw_status = static_cast<ULONG>(returnedFirmwareStatus);
        moduleParam.toStream(returnedOutputWriter);

        if (returnedFirmwareStatus == dsp_fw::IxcStatus::ADSP_IPC_SUCCESS) {

            /* Adding parameter content */
            returnedOutputWriter.writeRawBuffer(returnedParameterContent);
        }
    }

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(ioctlCode, &inputWriter.getBuffer(),
        &expectedOutputWriter.getBuffer(), &returnedOutputWriter.getBuffer());
}

template <typename FirmwareParameterType>
void MockedDeviceCommands::addGetModuleParameterCommand(
    uint16_t moduleId,
    uint16_t instanceId,
    uint32_t parameterTypeId,
    std::size_t expectedOutputBufferSize,
    const FirmwareParameterType &parameter,
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus)
{
    util::Buffer expectedOutput(expectedOutputBufferSize, 0xFF);

    util::ByteStreamWriter writer;
    parameter.toStream(writer);

    addModuleParameterCommand(
        Command::Get,
        moduleId,
        instanceId,
        parameterTypeId,
        expectedOutput,
        writer.getBuffer(),
        ioctlSuccess,
        returnedDriverStatus,
        returnedFirmwareStatus);
}

void MockedDeviceCommands::addLogParameterCommand(
    Command command,
    const driver::IoctlFwLogsState &inputFwParams,
    const driver::IoctlFwLogsState &outputFwParams,
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus)
{
    /* Creating ioctl expected buffer */
    util::ByteStreamWriter expectedWriter;

    /* Intc_App_TinyCmd structure */
    driver::Intc_App_TinyCmd tinyCmd(static_cast<ULONG>(driver::IOCTL_FEATURE::FEATURE_FW_LOGS),
        driver::logParametersCommandparameterId);
    tinyCmd.toStream(expectedWriter);

    /* IoctlFwLogsState structure*/
    inputFwParams.toStream(expectedWriter);

    uint32_t ioctlCode = command == Command::Get ?
    IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET : IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET;

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(ioctlCode, &expectedWriter.getBuffer(),
            &expectedWriter.getBuffer());
        return;
    }

    /* Creating ioctl returned buffer */
    util::ByteStreamWriter returnedWriter;

    /* Intc_App_TinyCmd structure */
    tinyCmd.Body.Status = returnedDriverStatus;
    tinyCmd.toStream(returnedWriter);

    if (NT_SUCCESS(returnedDriverStatus)) {

        /* IoctlFwLogsState structure*/
        outputFwParams.toStream(returnedWriter);
    }

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(ioctlCode, &expectedWriter.getBuffer(),
        &expectedWriter.getBuffer(), &returnedWriter.getBuffer());
}

void MockedDeviceCommands::addTlvParameterCommand(bool ioctlSuccess,
                                                  NTSTATUS returnedDriverStatus,
                                                  dsp_fw::IxcStatus returnedFirmwareStatus,
                                                  const Buffer &tlvList,
                                                  dsp_fw::BaseFwParams parameterId)
{
    util::Buffer expectedOutput(ModuleHandler::cavsTlvBufferSize, 0xFF);
    util::Buffer returnedOutput;
    returnedOutput = tlvList;

    addModuleParameterCommand(
        Command::Get,
        driver::baseFirwareModuleId,
        driver::baseFirwareInstanceId,
        static_cast<uint32_t>(parameterId),
        expectedOutput,
        returnedOutput,
        ioctlSuccess,
        returnedDriverStatus,
        returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetFwConfigCommand(bool ioctlSuccess,
                                                 NTSTATUS returnedDriverStatus,
                                                 dsp_fw::IxcStatus returnedFirmwareStatus,
                                                 const Buffer &fwConfigTlvList)
{
    addTlvParameterCommand(ioctlSuccess,
                           returnedDriverStatus,
                           returnedFirmwareStatus,
                           fwConfigTlvList,
                           dsp_fw::BaseFwParams::FW_CONFIG);
}

void MockedDeviceCommands::addGetHwConfigCommand(bool ioctlSuccess,
                                                 NTSTATUS returnedDriverStatus,
                                                 dsp_fw::IxcStatus returnedFirmwareStatus,
                                                 const Buffer &hwConfigTlvList)
{
    addTlvParameterCommand(ioctlSuccess,
                           returnedDriverStatus,
                           returnedFirmwareStatus,
                           hwConfigTlvList,
                           dsp_fw::BaseFwParams::HW_CONFIG_GET);
}

void MockedDeviceCommands::addGetModuleEntriesCommand(bool ioctlSuccess,
    NTSTATUS returnedDriverStatus, dsp_fw::IxcStatus returnedFirmwareStatus,
    uint32_t moduleCount, const std::vector<dsp_fw::ModuleEntry> &returnedEntries)
{
    std::size_t moduleInfoSize = dsp_fw::ModulesInfo::getAllocationSize(moduleCount);

    dsp_fw::ModulesInfo modulesInfo;
    modulesInfo.module_info = returnedEntries;

    addGetModuleParameterCommand(driver::baseFirwareModuleId,
        driver::baseFirwareInstanceId,
        static_cast<uint32_t>(dsp_fw::BaseFwParams::MODULES_INFO_GET),
        moduleInfoSize,
        modulesInfo, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

/* log parameters methods */
void MockedDeviceCommands::addGetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
    const driver::IoctlFwLogsState &returnedState)
{
    driver::IoctlFwLogsState expectedLogState = {
        static_cast<driver::IOCTL_LOG_STATE>(0xFFFFFFFF),
        static_cast<driver::FW_LOG_LEVEL>(0xFFFFFFFF),
        static_cast<driver::FW_LOG_OUTPUT>(0xFFFFFFFF),
    };

    addLogParameterCommand(Command::Get, expectedLogState, returnedState, ioctlSuccess,
        returnedStatus);
}

void MockedDeviceCommands::addSetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
    const driver::IoctlFwLogsState &expectedState)
{
    addLogParameterCommand(Command::Set, expectedState, expectedState, ioctlSuccess,
        returnedStatus);
}

void MockedDeviceCommands::addGetPipelineListCommand(
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus,
    uint32_t maxPplCount,
    const std::vector<uint32_t> &pipelineIds)
{
    std::size_t parameterSize = dsp_fw::PipelinesListInfo::getAllocationSize(maxPplCount);

    dsp_fw::PipelinesListInfo pipelinesListInfo;
    pipelinesListInfo.ppl_id = pipelineIds;

    addGetModuleParameterCommand(driver::baseFirwareModuleId,
        driver::baseFirwareInstanceId,
        static_cast<uint32_t>(dsp_fw::BaseFwParams::PIPELINE_LIST_INFO_GET),
        parameterSize,
        pipelinesListInfo, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetPipelinePropsCommand(
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus,
    uint32_t pipelineId,
    const dsp_fw::PplProps &props)
{
    /* Using extended parameter id to supply the pipeline id*/
    uint32_t paramId = ModuleHandler::getExtendedParameterId(
        dsp_fw::BaseFwParams::PIPELINE_PROPS_GET, pipelineId);

    addGetModuleParameterCommand(driver::baseFirwareModuleId,
        driver::baseFirwareInstanceId,
        paramId,
        ModuleHandler::maxParameterPayloadSize,
        props, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetSchedulersInfoCommand(
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus,
    uint32_t coreId,
    const dsp_fw::SchedulersInfo &info)
{
    /* Using extended parameter id to supply the core id*/
    uint32_t paramId = ModuleHandler::getExtendedParameterId(
        dsp_fw::BaseFwParams::SCHEDULERS_INFO_GET,
        coreId);

    addGetModuleParameterCommand(driver::baseFirwareModuleId,
        driver::baseFirwareInstanceId,
        paramId,
        ModuleHandler::maxParameterPayloadSize,
        info, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetGatewaysCommand(
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus,
    uint32_t gatewayCount,
    const std::vector<dsp_fw::GatewayProps> &gateways)
{
    /* Calculating the memory space required */
    std::size_t parameterSize = dsp_fw::GatewaysInfo::getAllocationSize(gatewayCount);

    dsp_fw::GatewaysInfo gatewaysInfo;
    gatewaysInfo.gateways = gateways;

    addGetModuleParameterCommand(driver::baseFirwareModuleId,
        driver::baseFirwareInstanceId,
        static_cast<uint32_t>(dsp_fw::BaseFwParams::GATEWAYS_INFO_GET),
        parameterSize,
        gatewaysInfo, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetModuleInstancePropsCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId, uint16_t instanceId,
    const dsp_fw::ModuleInstanceProps &props)
{
    addGetModuleParameterCommand(
        moduleId,
        instanceId,
        static_cast<uint32_t>(dsp_fw::BaseModuleParams::MOD_INST_PROPS),
        ModuleHandler::maxParameterPayloadSize,
        props, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addSetModuleParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
    const util::Buffer &parameterPayload)
{
    addModuleParameterCommand(
        Command::Set,
        moduleId,
        instanceId,
        parameterId,
        parameterPayload,
        parameterPayload,
        ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetModuleParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
    dsp_fw::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
    const util::Buffer &parameterPayload)
{
    addModuleParameterCommand(
        Command::Get,
        moduleId,
        instanceId,
        parameterId,
        util::Buffer(ModuleHandler::maxParameterPayloadSize, 0xFF),
        parameterPayload,
        ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

}
}
}
