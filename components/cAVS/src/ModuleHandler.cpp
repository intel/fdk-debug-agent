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
#include "Util/ByteStreamWriter.hpp"
#include "Tlv/TlvUnpack.hpp"

namespace debug_agent
{
namespace cavs
{

ModuleHandler::ModuleHandler(std::unique_ptr<ModuleHandlerImpl> impl)
    : mImpl(std::move(impl)),
      mFwConfig(readTlvParameters<dsp_fw::FwConfig>(dsp_fw::BaseFwParams::FW_CONFIG)),
      mHwConfig(readTlvParameters<dsp_fw::HwConfig>(dsp_fw::BaseFwParams::HW_CONFIG_GET))
{
    std::string error;
    if (not mFwConfig.isFwVersionValid) {
        error += "FW version\n";
    }
    if (not mFwConfig.isModulesCountValid) {
        error += "Module count\n";
    }
    if (not mFwConfig.isMaxPplCountValid) {
        error += "Max pipeline count\n";
    }
    if (not mFwConfig.isMaxModInstCountValid) {
        error += "Max module instance count\n";
    }
    if (not mHwConfig.isGatewayCountValid) {
        error += "Gateway count\n";
    }
    if (not mHwConfig.isDspCoreCountValid) {
        error += "DSP core count\n";
    }
    if (not error.empty()) {
        throw Exception("The following config items could not be retrieved from hardware:\n" +
                        error);
    }
    cacheModuleEntries();
}

util::Buffer ModuleHandler::configGet(uint16_t moduleId, uint16_t instanceId,
                                      dsp_fw::ParameterId parameterId, size_t parameterSize)
{
    try {
        return mImpl->configGet(moduleId, instanceId, parameterId, parameterSize);
    } catch (ModuleHandlerImpl::Exception &e) {
        throw Exception(e.what());
    }
}

void ModuleHandler::configSet(uint16_t moduleId, uint16_t instanceId,
                              dsp_fw::ParameterId parameterId, const util::Buffer &parameterPayload)
{
    try {
        mImpl->configSet(moduleId, instanceId, parameterId, parameterPayload);
    } catch (ModuleHandlerImpl::Exception &e) {
        throw Exception(e.what());
    }
}

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
TlvResponseHandlerInterface ModuleHandler::readTlvParameters(dsp_fw::BaseFwParams parameterId)
{
    TlvResponseHandlerInterface responseHandler;

    /** According to the SwAS, setting initial buffer size to tlvBufferSize.
     * Using 0xFF for test purpose (mark unused memory) */
    util::Buffer buffer = configGet(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
                                    dsp_fw::toParameterId(parameterId), tlvBufferSize);

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

    return responseHandler;
}

const std::vector<dsp_fw::ModuleEntry> &ModuleHandler::getModuleEntries() const noexcept
{
    return mModuleEntries;
}

const dsp_fw::ModuleEntry &ModuleHandler::findModuleEntry(uint16_t moduleId) const
{
    // TODO: have a map instead of a list if there's any performance issue?
    for (auto &module : mModuleEntries) {
        if (module.module_id == moduleId)
            return module;
    }
    throw Exception("module with id '" + std::to_string(moduleId) + "' not found");
}

const dsp_fw::ModuleEntry &ModuleHandler::findModuleEntry(const std::string &name) const
{
    for (auto &module : mModuleEntries) {
        if (module.getName() == name)
            return module;
    }
    throw Exception("module with name  '" + name + "' not found");
}

void ModuleHandler::cacheModuleEntries()
{
    auto moduleCount = mFwConfig.modulesCount;
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

    mModuleEntries = modulesInfo.module_info;
}

const dsp_fw::FwConfig &ModuleHandler::getFwConfig() const noexcept
{
    return mFwConfig;
}

const dsp_fw::HwConfig &ModuleHandler::getHwConfig() const noexcept
{
    return mHwConfig;
}

std::vector<dsp_fw::PipeLineIdType> ModuleHandler::getPipelineIdList()
{
    auto maxPplCount = mFwConfig.maxPplCount;
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

    return pipelineListInfo.ppl_id;
}

dsp_fw::PplProps ModuleHandler::getPipelineProps(dsp_fw::PipeLineIdType pipelineId)
{
    /* Using extended parameter id to supply the pipeline id*/
    auto paramId = getExtendedParameterId(dsp_fw::BaseFwParams::PIPELINE_PROPS_GET, pipelineId);

    dsp_fw::PplProps props;
    /* Query FW through driver */
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId, paramId,
                        maxParameterPayloadSize, props);

    return props;
}

dsp_fw::SchedulersInfo ModuleHandler::getSchedulersInfo(dsp_fw::CoreId coreId)
{
    /* Using extended parameter id to supply the core id*/
    auto paramId = getExtendedParameterId(dsp_fw::BaseFwParams::SCHEDULERS_INFO_GET, coreId);

    dsp_fw::SchedulersInfo schedulers;
    /* Query FW through driver */
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId, paramId,
                        maxParameterPayloadSize, schedulers);

    return schedulers;
}

std::vector<dsp_fw::GatewayProps> ModuleHandler::getGatewaysInfo()
{
    auto gatewayCount = mHwConfig.gatewayCount;
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

    return gatewaysInfo.gateways;
}

// TODO: use the dsp_fw-provided enum once available
void ModuleHandler::setPerfState(uint32_t state)
{
    util::MemoryByteStreamWriter writer;
    writer.write(state);
    configSet(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
              dsp_fw::ParameterId{dsp_fw::BaseFwParams::PERF_MEASUREMENTS_STATE},
              writer.getBuffer());
}

// TODO: use the dsp_fw-provided enum once available
uint32_t ModuleHandler::getPerfState()
{
    uint32_t fromDriver;
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
                        dsp_fw::toParameterId(dsp_fw::BaseFwParams::PERF_MEASUREMENTS_STATE),
                        sizeof(fromDriver), fromDriver);
    return fromDriver;
}

std::vector<dsp_fw::PerfDataItem> ModuleHandler::getPerfItems()
{
    auto itemCount = mFwConfig.maxModInstCount + mHwConfig.dspCoreCount;
    /* Calculating the memory space required */
    std::size_t parameterSize = dsp_fw::GlobalPerfData::getAllocationSize(itemCount);

    /* Query FW through driver */
    dsp_fw::GlobalPerfData perfData;
    getFwParameterValue(dsp_fw::baseFirmwareModuleId, dsp_fw::baseFirmwareInstanceId,
                        dsp_fw::toParameterId(dsp_fw::BaseFwParams::GLOBAL_PERF_DATA),
                        parameterSize, perfData);

    /* Checking returned perf item count */
    if (perfData.items.size() > itemCount) {
        throw Exception("Firmware has returned an invalid performance entries count: " +
                        std::to_string(perfData.items.size()) + " max is: " +
                        std::to_string(itemCount));
    }

    return perfData.items;
}

dsp_fw::GlobalMemoryState ModuleHandler::getGlobalMemoryState()
{
    return readTlvParameters<dsp_fw::GlobalMemoryState>(
        dsp_fw::BaseFwParams::MEMORY_STATE_INFO_GET);
}

dsp_fw::ModuleInstanceProps ModuleHandler::getModuleInstanceProps(uint16_t moduleId,
                                                                  uint16_t instanceId)
{
    dsp_fw::ModuleInstanceProps props;
    getFwParameterValue(moduleId, instanceId,
                        dsp_fw::toParameterId(dsp_fw::BaseModuleParams::MOD_INST_PROPS),
                        maxParameterPayloadSize, props);

    return props;
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

util::Buffer ModuleHandler::getModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                               dsp_fw::ParameterId parameterId,
                                               size_t parameterSize)
{
    /* Query FW through driver */
    return configGet(moduleId, instanceId, parameterId, parameterSize);
}
}
}
