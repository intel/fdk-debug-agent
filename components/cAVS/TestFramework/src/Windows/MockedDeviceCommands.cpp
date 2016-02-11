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
    Command command, uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterTypeId,
    const util::Buffer &expectedParameterContent, const util::Buffer &returnedParameterContent,
    bool ioctlSuccess, NTSTATUS returnedDriverStatus, dsp_fw::IxcStatus returnedFirmwareStatus)
{
    /* Expected output buffer*/
    util::ByteStreamWriter expectedOutputWriter;

    /* Adding driver Intc_App_Cmd_Body structure */
    driver::Intc_App_Cmd_Body bodyCmd;
    expectedOutputWriter.write(bodyCmd);

    /* Adding driver IoctlFwModuleParam structure */
    driver::IoctlFwModuleParam moduleParam(moduleId, instanceId, parameterTypeId,
                                           static_cast<uint32_t>(expectedParameterContent.size()));
    expectedOutputWriter.write(moduleParam);

    /* Adding parameter content */
    expectedOutputWriter.writeRawBuffer(expectedParameterContent);

    /* Creating the ioctl input buffer*/
    util::ByteStreamWriter inputWriter;

    /* Adding driver Intc_App_Cmd_Header structure */
    driver::Intc_App_Cmd_Header ioctlInput(
        static_cast<uint32_t>(driver::IOCTL_FEATURE::FEATURE_FW_MODULE_PARAM),
        driver::moduleParameterAccessCommandParameterId,
        static_cast<uint32_t>(expectedOutputWriter.getBuffer().size()));
    inputWriter.write(ioctlInput);

    uint32_t ioctlCode = command == Command::Get ? IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET
                                                 : IOCTL_CMD_APP_TO_AUDIODSP_BIG_SET;

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(ioctlCode, &inputWriter.getBuffer(),
                                    &expectedOutputWriter.getBuffer());
        return;
    }

    /* Expected returned buffer*/
    util::ByteStreamWriter returnedOutputWriter;

    /* Adding driver Intc_App_Cmd_Body structure */
    bodyCmd.Status = returnedDriverStatus;
    returnedOutputWriter.write(bodyCmd);

    if (NT_SUCCESS(returnedDriverStatus)) {

        /* Adding driver IoctlFwModuleParam structure */
        moduleParam.fw_status = static_cast<ULONG>(returnedFirmwareStatus);
        returnedOutputWriter.write(moduleParam);

        if (returnedFirmwareStatus == dsp_fw::IxcStatus::ADSP_IPC_SUCCESS) {

            /* Adding parameter content */
            returnedOutputWriter.writeRawBuffer(returnedParameterContent);
        }
    }

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(ioctlCode, &inputWriter.getBuffer(),
                                    &expectedOutputWriter.getBuffer(),
                                    &returnedOutputWriter.getBuffer());
}

template <typename FirmwareParameterType>
void MockedDeviceCommands::addGetModuleParameterCommand(
    uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterTypeId,
    std::size_t expectedOutputBufferSize, const FirmwareParameterType &parameter, bool ioctlSuccess,
    NTSTATUS returnedDriverStatus, dsp_fw::IxcStatus returnedFirmwareStatus)
{
    util::Buffer expectedOutput(expectedOutputBufferSize, 0xFF);

    util::ByteStreamWriter writer;
    writer.write(parameter);

    addModuleParameterCommand(Command::Get, moduleId, instanceId, parameterTypeId, expectedOutput,
                              writer.getBuffer(), ioctlSuccess, returnedDriverStatus,
                              returnedFirmwareStatus);
}

template <class IoCtlDescription>
void MockedDeviceCommands::addTinyCommand(typename const IoCtlDescription::Data &inputDriverStruct,
                                          typename const IoCtlDescription::Data &outputDriverStruct,
                                          bool ioctlSuccess, NTSTATUS returnedDriverStatus)
{
    static_assert(IoCtlDescription::type == driver::IoCtlType::TinyGet ||
                      IoCtlDescription::type == driver::IoCtlType::TinySet,
                  "For now, MockedDeviceCommands only supports Tiny ioctls");

    /* First creating expected body to know its serialized size */
    util::ByteStreamWriter expectedBodyWriter;
    driver::Intc_App_Cmd_Body body;
    expectedBodyWriter.write(body);
    expectedBodyWriter.write(inputDriverStruct);
    ULONG bodySize = static_cast<ULONG>(expectedBodyWriter.getBuffer().size());

    /* Creating header */
    driver::Intc_App_Cmd_Header header(IoCtlDescription::feature, IoCtlDescription::id, bodySize);

    /* Creating ioctl expected buffer containing header + body */
    util::ByteStreamWriter expectedWriter;
    expectedWriter.write(header);
    expectedWriter.writeRawBuffer(expectedBodyWriter.getBuffer());

    uint32_t ioctlCode = IoCtlDescription::type == driver::IoCtlType::TinyGet
                             ? IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET
                             : IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET;

    if (!ioctlSuccess) {
        mDevice.addFailedIoctlEntry(ioctlCode, &expectedWriter.getBuffer(),
                                    &expectedWriter.getBuffer());
        return;
    }

    /* Creating ioctl returned buffer */
    util::ByteStreamWriter returnedWriter;

    /* Putting same header */
    returnedWriter.write(header);

    /* Putting body with error code set*/
    body.Status = returnedDriverStatus;
    returnedWriter.write(body);

    if (NT_SUCCESS(returnedDriverStatus)) {

        /* IoctlFwLogsState structure*/
        returnedWriter.write(outputDriverStruct);
    }

    /* Adding entry */
    mDevice.addSuccessfulIoctlEntry(ioctlCode, &expectedWriter.getBuffer(),
                                    &expectedWriter.getBuffer(), &returnedWriter.getBuffer());
}

template <driver::IOCTL_FEATURE feature, uint32_t parameter, typename DriverStructure>
void MockedDeviceCommands::addTinyGetCommand(const DriverStructure &returnedDriverStruct,
                                             bool ioctlSuccess, NTSTATUS returnedDriverStatus)
{
    DriverStructure inputDriverStruct;

    /* By convention unset memory areas passed through ioctl are filled with 0xFF */
    memset(&inputDriverStruct, 0xFF, sizeof(DriverStructure));

    using T = IoCtlDescription<driver::IoCtlType::TinyGet, feature, parameter, DriverStructure>;
    addTinyCommand<T>(inputDriverStruct, returnedDriverStruct, ioctlSuccess, returnedDriverStatus);
}

template <driver::IOCTL_FEATURE feature, uint32_t parameter, typename DriverStructure>
void MockedDeviceCommands::addTinySetCommand(const DriverStructure &inputDriverStruct,
                                             bool ioctlSuccess, NTSTATUS returnedDriverStatus)
{
    DriverStructure returnedDriverStructure(inputDriverStruct);

    using T = IoCtlDescription<driver::IoCtlType::TinySet, feature, parameter, DriverStructure>;
    addTinyCommand<T>(inputDriverStruct, returnedDriverStructure, ioctlSuccess,
                      returnedDriverStatus);
}

void MockedDeviceCommands::addTlvParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                                  dsp_fw::IxcStatus returnedFirmwareStatus,
                                                  const Buffer &tlvList,
                                                  dsp_fw::BaseFwParams parameterId)
{
    util::Buffer expectedOutput(ModuleHandler::cavsTlvBufferSize, 0xFF);
    util::Buffer returnedOutput;
    returnedOutput = tlvList;

    addModuleParameterCommand(Command::Get, dsp_fw::baseFirwareModuleId,
                              dsp_fw::baseFirwareInstanceId, dsp_fw::toParameterId(parameterId),
                              expectedOutput, returnedOutput, ioctlSuccess, returnedDriverStatus,
                              returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetFwConfigCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                                 dsp_fw::IxcStatus returnedFirmwareStatus,
                                                 const Buffer &fwConfigTlvList)
{
    addTlvParameterCommand(ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus,
                           fwConfigTlvList, dsp_fw::BaseFwParams::FW_CONFIG);
}

void MockedDeviceCommands::addGetHwConfigCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                                 dsp_fw::IxcStatus returnedFirmwareStatus,
                                                 const Buffer &hwConfigTlvList)
{
    addTlvParameterCommand(ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus,
                           hwConfigTlvList, dsp_fw::BaseFwParams::HW_CONFIG_GET);
}

void MockedDeviceCommands::addGetModuleEntriesCommand(
    bool ioctlSuccess, NTSTATUS returnedDriverStatus, dsp_fw::IxcStatus returnedFirmwareStatus,
    uint32_t moduleCount, const std::vector<dsp_fw::ModuleEntry> &returnedEntries)
{
    std::size_t moduleInfoSize = dsp_fw::ModulesInfo::getAllocationSize(moduleCount);

    dsp_fw::ModulesInfo modulesInfo;
    modulesInfo.module_info = returnedEntries;

    addGetModuleParameterCommand(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
                                 dsp_fw::toParameterId(dsp_fw::BaseFwParams::MODULES_INFO_GET),
                                 moduleInfoSize, modulesInfo, ioctlSuccess, returnedDriverStatus,
                                 returnedFirmwareStatus);
}

/* log parameters methods */
void MockedDeviceCommands::addGetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
                                                      const driver::IoctlFwLogsState &returnedState)
{
    addTinyGetCommand<driver::IOCTL_FEATURE::FEATURE_FW_LOGS,
                      driver::logParametersCommandparameterId, driver::IoctlFwLogsState>(
        returnedState, ioctlSuccess, returnedStatus);
}

void MockedDeviceCommands::addSetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
                                                      const driver::IoctlFwLogsState &expectedState)
{
    addTinySetCommand<driver::IOCTL_FEATURE::FEATURE_FW_LOGS,
                      driver::logParametersCommandparameterId, driver::IoctlFwLogsState>(
        expectedState, ioctlSuccess, returnedStatus);
}

void MockedDeviceCommands::addGetProbeStateCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
                                                   driver::ProbeState returnedState)
{
    addTinyGetCommand<driver::IOCTL_FEATURE::FEATURE_PROBE_CAPTURE, 0, driver::ProbeState>(
        returnedState, ioctlSuccess, returnedStatus);
}

void MockedDeviceCommands::addSetProbeStateCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
                                                   driver::ProbeState expectedState)
{
    addTinySetCommand<driver::IOCTL_FEATURE::FEATURE_PROBE_CAPTURE, 0, driver::ProbeState>(
        expectedState, ioctlSuccess, returnedStatus);
}

void MockedDeviceCommands::addGetProbeConfigurationCommand(
    bool ioctlSuccess, NTSTATUS returnedStatus,
    const driver::ProbePointConfiguration &returnedConfiguration)
{
    addTinyGetCommand<driver::IOCTL_FEATURE::FEATURE_PROBE_CAPTURE, 1,
                      driver::ProbePointConfiguration>(returnedConfiguration, ioctlSuccess,
                                                       returnedStatus);
}

void MockedDeviceCommands::addSetProbeConfigurationCommand(
    bool ioctlSuccess, NTSTATUS returnedStatus,
    const driver::ProbePointConfiguration &expectedConfiguration)
{
    addTinySetCommand<driver::IOCTL_FEATURE::FEATURE_PROBE_CAPTURE, 1,
                      driver::ProbePointConfiguration>(expectedConfiguration, ioctlSuccess,
                                                       returnedStatus);
}

void MockedDeviceCommands::addGetPipelineListCommand(
    bool ioctlSuccess, NTSTATUS returnedDriverStatus, dsp_fw::IxcStatus returnedFirmwareStatus,
    uint32_t maxPplCount, const std::vector<dsp_fw::PipeLineIdType> &pipelineIds)
{
    std::size_t parameterSize = dsp_fw::PipelinesListInfo::getAllocationSize(maxPplCount);

    dsp_fw::PipelinesListInfo pipelinesListInfo;
    pipelinesListInfo.ppl_id = pipelineIds;

    addGetModuleParameterCommand(
        dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
        dsp_fw::toParameterId(dsp_fw::BaseFwParams::PIPELINE_LIST_INFO_GET), parameterSize,
        pipelinesListInfo, ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetPipelinePropsCommand(bool ioctlSuccess,
                                                      NTSTATUS returnedDriverStatus,
                                                      dsp_fw::IxcStatus returnedFirmwareStatus,
                                                      dsp_fw::PipeLineIdType pipelineId,
                                                      const dsp_fw::PplProps &props)
{
    /* Using extended parameter id to supply the pipeline id*/
    auto paramId =
        ModuleHandler::getExtendedParameterId(dsp_fw::BaseFwParams::PIPELINE_PROPS_GET, pipelineId);

    addGetModuleParameterCommand(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
                                 paramId, ModuleHandler::maxParameterPayloadSize, props,
                                 ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetSchedulersInfoCommand(bool ioctlSuccess,
                                                       NTSTATUS returnedDriverStatus,
                                                       dsp_fw::IxcStatus returnedFirmwareStatus,
                                                       dsp_fw::CoreId coreId,
                                                       const dsp_fw::SchedulersInfo &info)
{
    /* Using extended parameter id to supply the core id*/
    auto paramId =
        ModuleHandler::getExtendedParameterId(dsp_fw::BaseFwParams::SCHEDULERS_INFO_GET, coreId);

    addGetModuleParameterCommand(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
                                 paramId, ModuleHandler::maxParameterPayloadSize, info,
                                 ioctlSuccess, returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetGatewaysCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
                                                 dsp_fw::IxcStatus returnedFirmwareStatus,
                                                 uint32_t gatewayCount,
                                                 const std::vector<dsp_fw::GatewayProps> &gateways)
{
    /* Calculating the memory space required */
    std::size_t parameterSize = dsp_fw::GatewaysInfo::getAllocationSize(gatewayCount);

    dsp_fw::GatewaysInfo gatewaysInfo;
    gatewaysInfo.gateways = gateways;

    addGetModuleParameterCommand(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
                                 dsp_fw::toParameterId(dsp_fw::BaseFwParams::GATEWAYS_INFO_GET),
                                 parameterSize, gatewaysInfo, ioctlSuccess, returnedDriverStatus,
                                 returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetModuleInstancePropsCommand(
    bool ioctlSuccess, NTSTATUS returnedDriverStatus, dsp_fw::IxcStatus returnedFirmwareStatus,
    uint16_t moduleId, uint16_t instanceId, const dsp_fw::ModuleInstanceProps &props)
{
    addGetModuleParameterCommand(moduleId, instanceId,
                                 dsp_fw::toParameterId(dsp_fw::BaseModuleParams::MOD_INST_PROPS),
                                 ModuleHandler::maxParameterPayloadSize, props, ioctlSuccess,
                                 returnedDriverStatus, returnedFirmwareStatus);
}

void MockedDeviceCommands::addSetModuleParameterCommand(bool ioctlSuccess,
                                                        NTSTATUS returnedDriverStatus,
                                                        dsp_fw::IxcStatus returnedFirmwareStatus,
                                                        uint16_t moduleId, uint16_t instanceId,
                                                        dsp_fw::ParameterId parameterId,
                                                        const util::Buffer &parameterPayload)
{
    addModuleParameterCommand(Command::Set, moduleId, instanceId, parameterId, parameterPayload,
                              parameterPayload, ioctlSuccess, returnedDriverStatus,
                              returnedFirmwareStatus);
}

void MockedDeviceCommands::addGetModuleParameterCommand(bool ioctlSuccess,
                                                        NTSTATUS returnedDriverStatus,
                                                        dsp_fw::IxcStatus returnedFirmwareStatus,
                                                        uint16_t moduleId, uint16_t instanceId,
                                                        dsp_fw::ParameterId parameterId,
                                                        const util::Buffer &parameterPayload)
{
    addModuleParameterCommand(Command::Get, moduleId, instanceId, parameterId,
                              util::Buffer(ModuleHandler::maxParameterPayloadSize, 0xFF),
                              parameterPayload, ioctlSuccess, returnedDriverStatus,
                              returnedFirmwareStatus);
}
}
}
}
