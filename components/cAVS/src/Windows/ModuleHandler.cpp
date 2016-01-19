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
#include "cAVS/Windows/ModuleHandler.hpp"
#include "cAVS/Windows/WindowsTypes.hpp"
#include "cAVS/Windows/IoctlHelpers.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Tlv/TlvUnpack.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

void ModuleHandler::bigCmdModuleAccessIoctl(bool isGet, uint16_t moduleId, uint16_t instanceId,
                                            dsp_fw::ParameterId moduleParamId,
                                            const util::Buffer &suppliedOutputBuffer,
                                            util::Buffer &returnedOutputBuffer)
{
    /* Creating the body payload using the IoctlFwModuleParam type */
    driver::IoctlFwModuleParam moduleParam(moduleId, instanceId, moduleParamId,
                                           static_cast<uint32_t>(suppliedOutputBuffer.size()));

    util::ByteStreamWriter bodyPayloadWriter;
    bodyPayloadWriter.write(moduleParam);
    bodyPayloadWriter.writeRawBuffer(suppliedOutputBuffer);

    /* Creating BigGet/Set ioctl buffers */
    util::Buffer headerBuffer;
    util::Buffer bodyBuffer;
    IoctlHelpers::toBigCmdBuffers(
        static_cast<uint32_t>(driver::IOCTL_FEATURE::FEATURE_FW_MODULE_PARAM),
        driver::moduleParameterAccessCommandParameterId, bodyPayloadWriter.getBuffer(),
        headerBuffer, bodyBuffer);

    /* Performing the io ctl */
    try {
        mDevice.ioControl(isGet ? IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET
                                : IOCTL_CMD_APP_TO_AUDIODSP_BIG_SET,
                          &headerBuffer, &bodyBuffer);
    } catch (Device::Exception &e) {
        throw Exception("Device returns an exception: " + std::string(e.what()));
    }

    /* Reading the result */
    try {
        NTSTATUS driverStatus;
        util::Buffer bodyPayloadBuffer;

        /* Parsing returned output buffer */
        IoctlHelpers::fromBigCmdBodyBuffer(bodyBuffer, driverStatus, bodyPayloadBuffer);

        /* Checking driver status */
        if (!NT_SUCCESS(driverStatus)) {
            throw Exception("Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(driverStatus)));
        }

        /* Reading driver IoctlFwModuleParam structure from body payload */
        util::ByteStreamReader reader(bodyPayloadBuffer);
        reader.read(moduleParam);

        /* Checking firwmare status */
        if (moduleParam.fw_status != static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS)) {
            throw Exception("Firmware returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(moduleParam.fw_status)));
        }

        returnedOutputBuffer.resize(reader.getBuffer().size() - reader.getPointerOffset());
        std::copy(reader.getBuffer().begin() + reader.getPointerOffset(), reader.getBuffer().end(),
                  returnedOutputBuffer.begin());
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Can not decode returned ioctl output buffer: " + std::string(e.what()));
    }
}

template <typename FirmwareParameterType>
void ModuleHandler::bigGetModuleAccessIoctl(uint16_t moduleId, uint16_t instanceId,
                                            dsp_fw::ParameterId moduleParamId,
                                            std::size_t fwParameterSize,
                                            FirmwareParameterType &result)
{
    util::Buffer suppliedBuffer(fwParameterSize, 0xFF);
    util::Buffer returnedBuffer;
    bigCmdModuleAccessIoctl(true, moduleId, instanceId, moduleParamId, suppliedBuffer,
                            returnedBuffer);

    util::ByteStreamReader reader(returnedBuffer);
    try {
        /* Reading parameter */
        reader.read(result);

        if (!reader.isEOS()) {
            /** @todo use logging or throw an exception */
            std::cout << "Fw parameter buffer has not been fully consumed,"
                      << " pointer=" << reader.getPointerOffset()
                      << " size=" << reader.getBuffer().size()
                      << " remaining= " << (reader.getBuffer().size() - reader.getBuffer().size());
        }
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Can not decode fw parameter: " + std::string(e.what()));
    }
}

void ModuleHandler::getModulesEntries(uint32_t moduleCount,
                                      std::vector<dsp_fw::ModuleEntry> &modulesEntries)
{
    std::size_t moduleInfoSize = dsp_fw::ModulesInfo::getAllocationSize(moduleCount);

    dsp_fw::ModulesInfo modulesInfo;
    bigGetModuleAccessIoctl(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
                            dsp_fw::toParameterId(dsp_fw::BaseFwParams::MODULES_INFO_GET),
                            moduleInfoSize, modulesInfo);

    /** @todo use logging */
    std::cout << "Number of modules found in FW: " << modulesInfo.module_info.size() << std::endl;

    if (modulesInfo.module_info.size() != moduleCount) {
        throw Exception("Firmware has returned an invalid module count: " +
                        std::to_string(modulesInfo.module_info.size()) + " instead of " +
                        std::to_string(moduleCount));
    }

    modulesEntries = modulesInfo.module_info;
}

template <typename TlvResponseHandlerInterface>
void ModuleHandler::readTlvParameters(TlvResponseHandlerInterface &responseHandler,
                                      dsp_fw::BaseFwParams parameterId)
{
    util::Buffer suppliedBuffer(cavsTlvBufferSize, 0xFF);
    util::Buffer returnedBuffer;
    bigCmdModuleAccessIoctl(true, dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
                            dsp_fw::toParameterId(parameterId), suppliedBuffer, returnedBuffer);

    /* Now parse the TLV answer */
    tlv::TlvUnpack unpack(responseHandler, returnedBuffer);

    bool end = false;
    do {

        try {
            end = !unpack.readNext();
        } catch (tlv::TlvUnpack::Exception &e) {
            /* @todo use log instead ! */
            std::cout << "Error while parsing TLV for base FW parameter "
                      << static_cast<uint32_t>(parameterId) << ": " << e.what() << std::endl;
        }
    } while (!end);
}

void ModuleHandler::getFwConfig(dsp_fw::FwConfig &fwConfig)
{
    readTlvParameters<dsp_fw::FwConfig>(fwConfig, dsp_fw::BaseFwParams::FW_CONFIG);
}

void ModuleHandler::getHwConfig(dsp_fw::HwConfig &hwConfig)
{
    readTlvParameters<dsp_fw::HwConfig>(hwConfig, dsp_fw::BaseFwParams::HW_CONFIG_GET);
}

void ModuleHandler::getPipelineIdList(uint32_t maxPplCount,
                                      std::vector<dsp_fw::PipeLineIdType> &pipelinesIds)
{
    /* Calculating the memory space required */
    std::size_t parameterSize = dsp_fw::PipelinesListInfo::getAllocationSize(maxPplCount);

    /* Performing ioctl*/
    dsp_fw::PipelinesListInfo pipelineListInfo;
    bigGetModuleAccessIoctl(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
                            dsp_fw::toParameterId(dsp_fw::BaseFwParams::PIPELINE_LIST_INFO_GET),
                            parameterSize, pipelineListInfo);

    /* Checking returned pipeline count */
    if (pipelineListInfo.ppl_id.size() > maxPplCount) {
        throw Exception("Firmware has returned an invalid pipeline count: " +
                        std::to_string(pipelineListInfo.ppl_id.size()) + " max is: " +
                        std::to_string(maxPplCount));
    }

    pipelinesIds = pipelineListInfo.ppl_id;
}

void ModuleHandler::getPipelineProps(dsp_fw::PipeLineIdType pipelineId, dsp_fw::PplProps &props)
{
    /* Using extended parameter id to supply the pipeline id*/
    auto paramId = getExtendedParameterId(dsp_fw::BaseFwParams::PIPELINE_PROPS_GET, pipelineId);

    /* Performing ioctl*/
    bigGetModuleAccessIoctl(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId, paramId,
                            maxParameterPayloadSize, props);
}

void ModuleHandler::getSchedulersInfo(dsp_fw::CoreId coreId, dsp_fw::SchedulersInfo &schedulers)
{
    /* Using extended parameter id to supply the core id*/
    auto paramId = getExtendedParameterId(dsp_fw::BaseFwParams::SCHEDULERS_INFO_GET, coreId);

    /* Performing ioctl*/
    bigGetModuleAccessIoctl(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId, paramId,
                            maxParameterPayloadSize, schedulers);
}

void ModuleHandler::getGatewaysInfo(uint32_t gatewayCount,
                                    std::vector<dsp_fw::GatewayProps> &gateways)
{
    /* Calculating the memory space required */
    std::size_t parameterSize = dsp_fw::GatewaysInfo::getAllocationSize(gatewayCount);

    /* Performing ioctl*/
    dsp_fw::GatewaysInfo gatewaysInfo;
    bigGetModuleAccessIoctl(dsp_fw::baseFirwareModuleId, dsp_fw::baseFirwareInstanceId,
                            dsp_fw::toParameterId(dsp_fw::BaseFwParams::GATEWAYS_INFO_GET),
                            parameterSize, gatewaysInfo);

    /* Checking returned gateway count */
    if (gatewaysInfo.gateways.size() > gatewayCount) {
        throw Exception("Firmware has returned an invalid gateway count: " +
                        std::to_string(gatewaysInfo.gateways.size()) + " max is: " +
                        std::to_string(gatewayCount));
    }

    gateways = gatewaysInfo.gateways;
}

void ModuleHandler::getModuleInstanceProps(uint16_t moduleId, uint16_t instanceId,
                                           dsp_fw::ModuleInstanceProps &props)
{
    /* Performing ioctl */
    bigGetModuleAccessIoctl(moduleId, instanceId,
                            dsp_fw::toParameterId(dsp_fw::BaseModuleParams::MOD_INST_PROPS),
                            maxParameterPayloadSize, props);
}

void ModuleHandler::setModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                       dsp_fw::ParameterId parameterId,
                                       const util::Buffer &parameterPayload)
{
    util::Buffer returnedBuffer;

    /* Performing ioctl */
    bigCmdModuleAccessIoctl(false, moduleId, instanceId, parameterId, parameterPayload,
                            returnedBuffer);
}

void ModuleHandler::getModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                       dsp_fw::ParameterId parameterId,
                                       util::Buffer &parameterPayload)
{
    util::Buffer suppliedBuffer(maxParameterPayloadSize, 0xFF);

    /* Performing ioctl */
    bigCmdModuleAccessIoctl(true, moduleId, instanceId, parameterId, suppliedBuffer,
                            parameterPayload);
}
}
}
}
