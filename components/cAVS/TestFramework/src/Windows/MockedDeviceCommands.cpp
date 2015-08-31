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
#include "cAVS/Windows/IoCtlStructureHelpers.hpp"
#include "cAVS/Windows/ModuleHandler.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

template <typename FirmwareParameterType>
void MockedDeviceCommands::addModuleParameterCommand(
    Command command,
    uint32_t parameterTypeId,
    const Buffer &returnedParameterContent,
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId,
    uint16_t instanceId)
{
    addModuleParameterCommand<FirmwareParameterType>(
        command,
        parameterTypeId,
        Buffer(returnedParameterContent.getSize()),
        returnedParameterContent,
        ioctlSuccess,
        returnedDriverStatus,
        returnedFirmwareStatus,
        moduleId,
        instanceId);
}

template <typename FirmwareParameterType>
void MockedDeviceCommands::addModuleParameterCommand(
    Command command,
    uint32_t parameterTypeId,
    const Buffer &expectedParameterContent,
    const Buffer &returnedParameterContent,
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId,
    uint16_t instanceId)
{
    uint32_t ioctlCode = command == Command::Get ?
        IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET : IOCTL_CMD_APP_TO_AUDIODSP_BIG_SET;

    /* Expected output buffer*/
    BigCmdModuleAccessIoctlOutput<FirmwareParameterType> expectedOutput(
        parameterTypeId, expectedParameterContent.getSize(), moduleId, instanceId);
    expectedOutput.setFirmwareParameterContent(expectedParameterContent.getElements());

    /* Filling expected input buffer */
    TypedBuffer<driver::Intc_App_Cmd_Header> expectedInput;
    expectedInput->FeatureID =
        static_cast<ULONG>(driver::FEATURE_FW_MODULE_PARAM);
    expectedInput->ParameterID = 0; /* only one parameter id for this feature */
    expectedInput->DataSize = static_cast<ULONG>(expectedOutput.getBuffer().getSize());

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(
            ioctlCode,
            &expectedInput,
            &expectedOutput.getBuffer());
        return;
    }

    /* Returned output buffer*/
    BigCmdModuleAccessIoctlOutput<FirmwareParameterType> returnedOutput(
        parameterTypeId, returnedParameterContent.getSize());

    returnedOutput.getCmdBody().Status = returnedDriverStatus;
    if (NT_SUCCESS(returnedDriverStatus)) {

        /* If the driver returns success, set the firmware status*/
        returnedOutput.getModuleParameterAccess().fw_status = returnedFirmwareStatus;

        if (returnedFirmwareStatus == dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS) {

            /* Setting returned parameter content if the firmware returns success */
            returnedOutput.setFirmwareParameterContent(returnedParameterContent.getElements());
        }
    }

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(ioctlCode, &expectedInput,
        &expectedOutput.getBuffer(), &returnedOutput.getBuffer());
}

void MockedDeviceCommands::addTlvParameterCommand(bool ioctlSuccess,
                                                  NTSTATUS returnedDriverStatus,
                                                  dsp_fw::Message::IxcStatus returnedFirmwareStatus,
                                                  const std::vector<char> &tlvList,
                                                  dsp_fw::BaseFwParams parameterId)
{
    Buffer expectedOutput(ModuleHandler::cavsTlvBufferSize);
    Buffer returnedOutput(tlvList);

    addModuleParameterCommand<char>(
        Command::Get,
        parameterId,
        expectedOutput,
        returnedOutput,
        ioctlSuccess,
        returnedDriverStatus,
        returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetFwConfigCommand(bool ioctlSuccess,
                                                 NTSTATUS returnedDriverStatus,
                                                 dsp_fw::Message::IxcStatus returnedFirmwareStatus,
                                                 const std::vector<char> &fwConfigTlvList)
{
    addTlvParameterCommand(ioctlSuccess,
                           returnedDriverStatus,
                           returnedFirmwareStatus,
                           fwConfigTlvList,
                           dsp_fw::FW_CONFIG);
}

void MockedDeviceCommands::addGetHwConfigCommand(bool ioctlSuccess,
                                                 NTSTATUS returnedDriverStatus,
                                                 dsp_fw::Message::IxcStatus returnedFirmwareStatus,
                                                 const std::vector<char> &hwConfigTlvList)
{
    addTlvParameterCommand(ioctlSuccess,
                           returnedDriverStatus,
                           returnedFirmwareStatus,
                           hwConfigTlvList,
                           dsp_fw::HW_CONFIG_GET);
}

void MockedDeviceCommands::addGetModuleEntriesCommand(bool ioctlSuccess,
    NTSTATUS returnedDriverStatus, dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    const std::vector<ModuleEntry> &returnedEntries)
{
    std::size_t moduleInfoSize = ModulesInfoHelper::getAllocationSize();

    /** Filling a ModulesInfo structure with the supplied module entries */
    TypedBuffer<dsp_fw::ModulesInfo> buffer(moduleInfoSize);
    buffer->module_count = static_cast<uint32_t>(returnedEntries.size());
    for (std::size_t i = 0; i < returnedEntries.size(); ++i) {
        buffer->module_info[i] = returnedEntries[i];
    }

    addModuleParameterCommand<dsp_fw::ModulesInfo>(Command::Get, dsp_fw::MODULES_INFO_GET,
        buffer, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

/* log parameters methods */

void MockedDeviceCommands::addGetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
    const driver::IoctlFwLogsState &returnedState)
{
    /* Expected buffer, used as both expected input AND output buffer */
    TinyCmdLogParameterIoctl expected;

    /* Returned output buffer*/
    TinyCmdLogParameterIoctl returned(expected);

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET, &expected.getBuffer(),
            &expected.getBuffer());
        return;
    }

    /* Result code */
    returned.getTinyCmd().Body.Status = returnedStatus;
    if (NT_SUCCESS(returnedStatus)) {

        /* Setting returned log state content if the driver returns success */
        returned.getFwLogsState() = returnedState;
    }

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET, &expected.getBuffer(),
        &expected.getBuffer(), &returned.getBuffer());
}

void MockedDeviceCommands::addSetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
    const driver::IoctlFwLogsState &expectedState)
{
    /* Expected buffer, used as both expected input AND output buffer */
    TinyCmdLogParameterIoctl expected;

    /* Setting expected log state content */
    expected.getFwLogsState() = expectedState;

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET, &expected.getBuffer(),
            &expected.getBuffer());
        return;
    }

    /* Returned output buffer*/
    TinyCmdLogParameterIoctl returned(expected);

    /* Result code */
    returned.getTinyCmd().Body.Status = returnedStatus;

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET, &expected.getBuffer(),
        &expected.getBuffer(), &returned.getBuffer());
}

void MockedDeviceCommands::addGetPipelineListCommand(
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint32_t maxPplCount,
    const std::vector<uint32_t> &pipelineIds)
{
    std::size_t parameterSize =
        MEMBER_SIZE(dsp_fw::PipelinesListInfo, ppl_count) +
        maxPplCount * MEMBER_SIZE(dsp_fw::PipelinesListInfo, ppl_id);

    /** Filling a PipelinesListInfo structure with the supplied pipeline ids */
    TypedBuffer<dsp_fw::PipelinesListInfo> buffer(parameterSize);
    buffer->ppl_count = static_cast<uint32_t>(pipelineIds.size());
    for (std::size_t i = 0; i < pipelineIds.size(); ++i) {
        buffer->ppl_id[i] = pipelineIds[i];
    }

    addModuleParameterCommand<dsp_fw::ModulesInfo>(Command::Get, dsp_fw::PIPELINE_LIST_INFO_GET,
        buffer, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetPipelinePropsCommand(
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint32_t pipelineId,
    const DSPplProps &props)
{
    /* Constructing expected output structure */
    Buffer expectedOutput(ModuleHandler::maxParameterPayloadSize);

    /* Filling input argument*/
    dsp_fw::PplPropsIn *pplPropsIn =
        reinterpret_cast<dsp_fw::PplPropsIn *>(expectedOutput.getPtr());
    pplPropsIn->ppl_id = pipelineId;

    /* Serializing Ppl props*/
    util::ByteStreamWriter writer;
    props.toStream(writer);

    /* Constructing returned output structure */
    Buffer returnedOutput(writer.getBuffer());

    addModuleParameterCommand<dsp_fw::ModulesInfo>(Command::Get, dsp_fw::PIPELINE_PROPS_GET,
        expectedOutput, returnedOutput, ioctlSuccess,
        returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetSchedulersInfoCommand(
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint32_t coreId,
    const DSSchedulersInfo &info)
{
    /* Constructing expected output structure */
    Buffer expectedOutput(ModuleHandler::maxParameterPayloadSize);

    /* Filling input argument*/
    dsp_fw::SchedulersInfoIn *schedIn =
        reinterpret_cast<dsp_fw::SchedulersInfoIn *>(expectedOutput.getPtr());
    schedIn->core_id = coreId;

    /* Serializing SchedulersInfo*/
    util::ByteStreamWriter writer;
    info.toStream(writer);

    /* Constructing returned output structure */
    Buffer returnedOutput(writer.getBuffer());

    addModuleParameterCommand<dsp_fw::ModulesInfo>(Command::Get, dsp_fw::SCHEDULERS_INFO_GET,
        expectedOutput, returnedOutput, ioctlSuccess,
        returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetGatewaysCommand(
    bool ioctlSuccess,
    NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint32_t gatewayCount,
    const std::vector<dsp_fw::GatewayProps> &gateways)
{
    /* Calculating the memory space required */
    std::size_t parameterSize =
        MEMBER_SIZE(dsp_fw::GatewaysInfo, gateway_count) +
        gatewayCount * MEMBER_SIZE(dsp_fw::GatewaysInfo, gateways);

    /** Filling a PipelinesListInfo structure with the supplied pipeline ids */
    TypedBuffer<dsp_fw::GatewaysInfo> buffer(parameterSize);
    buffer->gateway_count = static_cast<uint32_t>(gateways.size());
    for (std::size_t i = 0; i < gateways.size(); ++i) {
        buffer->gateways[i] = gateways[i];
    }

    addModuleParameterCommand<dsp_fw::ModulesInfo>(Command::Get, dsp_fw::GATEWAYS_INFO_GET,
        buffer, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetModuleInstancePropsCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId, uint16_t instanceId,
    const DSModuleInstanceProps &props)
{
    /* Constructing expected output structure */
    Buffer expectedOutput(ModuleHandler::maxParameterPayloadSize);

    /* Serializing SchedulersInfo*/
    util::ByteStreamWriter writer;
    props.toStream(writer);

    /* Constructing returned output structure */
    Buffer returnedOutput(writer.getBuffer());

    addModuleParameterCommand<dsp_fw::ModulesInfo>(Command::Get, dsp_fw::MOD_INST_PROPS,
        expectedOutput, returnedOutput, ioctlSuccess,
        returnedDriverStatus, returnedFirmwareStatus, moduleId, instanceId);
}

void MockedDeviceCommands::addSetModuleParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
    const std::vector<uint8_t> &parameterPayload)
{
    /* Constructing output structure */
    Buffer output(parameterPayload);

    addModuleParameterCommand<dsp_fw::ModulesInfo>(Command::Set, parameterId,
        output, output, ioctlSuccess,
        returnedDriverStatus, returnedFirmwareStatus, moduleId, instanceId);
}

void MockedDeviceCommands::addGetModuleParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
    dsp_fw::Message::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
    const std::vector<uint8_t> &parameterPayload)
{
    /* Constructing expected output structure */
    Buffer expectedOutput(ModuleHandler::maxParameterPayloadSize);

    /* Constructing returned output structure */
    Buffer returnedOutput(parameterPayload);

    addModuleParameterCommand<dsp_fw::ModulesInfo>(Command::Get, parameterId,
        expectedOutput, returnedOutput, ioctlSuccess,
        returnedDriverStatus, returnedFirmwareStatus, moduleId, instanceId);
}

}
}
}
