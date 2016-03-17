/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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
#include "cAVS/ModuleHandler.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Tlv/TlvUnpack.hpp"

namespace debug_agent
{
namespace cavs
{
/** In order to invoque this constant in std::min that requires reference, need
 * WA to force the compiler giving an address.
 */
const size_t ModuleHandler::maxParameterPayloadSize;

template <typename FirmwareParameterType>
void ModuleHandler::getFwParameterValue(uint16_t moduleId, uint16_t instanceId,
                                        dsp_fw::ParameterId moduleParamId,
                                        std::size_t fwParameterSize, FirmwareParameterType &result)
{
    util::Buffer buffer = configGet(moduleId, instanceId, moduleParamId, fwParameterSize);

    util::MemoryByteStreamReader reader(buffer);
    try {
        /* Reading parameter */
        reader.read(result);

        if (!reader.isEOS()) {
            /** @todo use logging or throw an exception */
            std::cout << "Fw parameter buffer has not been fully consumed,"
                      << " pointer=" << reader.getPointerOffset()
                      << " size=" << reader.getBuffer().size()
                      << " remaining= " << (reader.getBuffer().size() - reader.getPointerOffset())
                      << std::endl;
        }
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Can not decode fw parameter: " + std::string(e.what()));
    }
}

template <typename TlvResponseHandlerInterface>
void ModuleHandler::readTlvParameters(TlvResponseHandlerInterface &responseHandler,
                                      dsp_fw::BaseFwParams parameterId)
{
    /** According to the SwAS, setting initial buffer size to cavsTlvBufferSize.
     * Using 0xFF for test purpose (mark unused memory) */
    util::Buffer buffer = configGet(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
                                    dsp_fw::toParameterId(parameterId), cavsTlvBufferSize);

    /* Now parse the TLV answer */
    tlv::TlvUnpack unpack(responseHandler, buffer);

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

void ModuleHandler::getModulesEntries(uint32_t moduleCount,
                                      std::vector<dsp_fw::ModuleEntry> &modulesEntries)
{
    std::size_t moduleInfoSize = dsp_fw::ModulesInfo::getAllocationSize(moduleCount);

    dsp_fw::ModulesInfo modulesInfo;
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
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

    /* Query FW through driver */
    dsp_fw::PipelinesListInfo pipelineListInfo;
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
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

    /* Query FW through driver */
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId, paramId,
                        maxParameterPayloadSize, props);
}

void ModuleHandler::getSchedulersInfo(dsp_fw::CoreId coreId, dsp_fw::SchedulersInfo &schedulers)
{
    /* Using extended parameter id to supply the core id*/
    auto paramId = getExtendedParameterId(dsp_fw::BaseFwParams::SCHEDULERS_INFO_GET, coreId);

    /* Query FW through driver */
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId, paramId,
                        maxParameterPayloadSize, schedulers);
}

void ModuleHandler::getGatewaysInfo(uint32_t gatewayCount,
                                    std::vector<dsp_fw::GatewayProps> &gateways)
{
    /* Calculating the memory space required */
    std::size_t parameterSize = dsp_fw::GatewaysInfo::getAllocationSize(gatewayCount);

    /* Query FW through driver */
    dsp_fw::GatewaysInfo gatewaysInfo;
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
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
    getFwParameterValue(moduleId, instanceId,
                        dsp_fw::toParameterId(dsp_fw::BaseModuleParams::MOD_INST_PROPS),
                        maxParameterPayloadSize, props);
}

void ModuleHandler::setModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                       dsp_fw::ParameterId parameterId,
                                       const util::Buffer &parameterPayload)
{
    if (parameterPayload.size() > maxParameterPayloadSize) {
        throw Exception("Cannot set module parameter: payload to big : " +
                        std::to_string(parameterPayload.size()) + " max: " +
                        std::to_string(maxParameterPayloadSize));
    }

    configSet(moduleId, instanceId, parameterId, parameterPayload);
}

void ModuleHandler::getModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                       dsp_fw::ParameterId parameterId,
                                       util::Buffer &parameterPayload, size_t parameterSize)
{
    /* Query FW through driver */
    parameterPayload = configGet(moduleId, instanceId, parameterId, parameterSize);
}
}
}
