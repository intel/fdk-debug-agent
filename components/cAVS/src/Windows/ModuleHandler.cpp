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
#include "Util/ByteStreamReader.hpp"
#include "Tlv/TlvUnpack.hpp"
#include <vector>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

template <typename FirmwareParameterType>
void ModuleHandler::bigGetModuleAccessIoctl(
    BigCmdModuleAccessIoctlOutput<FirmwareParameterType> &output)
{
    /* Creating ioctl input structure */
    TypedBuffer<driver::Intc_App_Cmd_Header> ioctlInput;
    ioctlInput->FeatureID = static_cast<ULONG>(driver::FEATURE_FW_MODULE_PARAM);
    ioctlInput->ParameterID = moduleParameterAccessParameterId;
    ioctlInput->DataSize = static_cast<ULONG>(output.getBuffer().getSize());

    /* Performing the io ctl */
    try
    {
        mDevice.ioControl(IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET, &ioctlInput, &output.getBuffer());
    }
    catch (Device::Exception &e)
    {
        throw Exception("Device returns an exception: " + std::string(e.what()));
    }

    /* Checking driver status */
    if (!NT_SUCCESS(output.getCmdBody().Status))
    {
        throw Exception("Driver returns invalid status: " +
            std::to_string(static_cast<uint32_t>(output.getCmdBody().Status)));
    }

    /* Checking firwmare status */
    if (output.getModuleParameterAccess().fw_status !=
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS) {
        throw Exception("Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(output.getModuleParameterAccess().fw_status)));
    }
}

void ModuleHandler::getModulesEntries(std::vector<ModuleEntry> &modulesEntries)
{
    std::size_t moduleInfoSize = ModulesInfoHelper::getAllocationSize();

    /* Constructing ioctl output structure */
    BigCmdModuleAccessIoctlOutput<dsp_fw::ModulesInfo>
        ioctlOutput(dsp_fw::MODULES_INFO_GET, moduleInfoSize);

    /* Performing ioctl */
    bigGetModuleAccessIoctl<dsp_fw::ModulesInfo>(ioctlOutput);

    /* Checking returned module count */
    const dsp_fw::ModulesInfo &modulesInfo = ioctlOutput.getFirmwareParameter();

    /** @todo use logging */
    std::cout << "Number of modules found in FW: " << modulesInfo.module_count << std::endl;

    if (modulesInfo.module_count > dsp_fw::MaxModuleCount) {
        throw Exception("Firmware has returned an invalid module count: " +
            std::to_string(modulesInfo.module_count));
    }

    /* Retrieving module entries */
    for (std::size_t i = 0; i < modulesInfo.module_count; i++) {
        modulesEntries.push_back(modulesInfo.module_info[i]);
    }
}

template<typename TlvResponseHandlerInterface>
void ModuleHandler::readTlvParameters(TlvResponseHandlerInterface &responseHandler,
                                      dsp_fw::BaseFwParams parameterId)
{
    /* Constructing ioctl output structure*/
    BigCmdModuleAccessIoctlOutput<char> ioctlOutput(
        parameterId, cavsTlvBufferSize);

    /* Performing ioctl */
    bigGetModuleAccessIoctl<char>(ioctlOutput);

    /* Retrieving properties */
    size_t tlvBufferSize;
    const char * tlvBuffer = &ioctlOutput.getFirmwareParameter(tlvBufferSize);

    /* Now parse the TLV answer */
    tlv::TlvUnpack unpack(responseHandler, tlvBuffer, tlvBufferSize);

    bool end = false;
    do {

        try
        {
            end = !unpack.readNext();
        }
        catch (tlv::TlvUnpack::Exception &e)
        {
            /* @todo use log instead ! */
            std::cout << "Error while parsing TLV for base FW parameter "
                << parameterId << ": " << e.what() << std::endl;
        }
    } while (!end);

}

void ModuleHandler::getFwConfig(FwConfig &fwConfig)
{
    readTlvParameters<FwConfig>(fwConfig, dsp_fw::FW_CONFIG);
}

void ModuleHandler::getHwConfig(HwConfig &hwConfig)
{
    readTlvParameters<HwConfig>(hwConfig, dsp_fw::HW_CONFIG_GET);
}

void ModuleHandler::getPipelineIdList(uint32_t maxPplCount, std::vector<uint32_t> &pipelinesIds)
{
    /* Calculating the memory space required */
    std::size_t parameterSize =
        MEMBER_SIZE(dsp_fw::PipelinesListInfo, ppl_count) +
        maxPplCount * MEMBER_SIZE(dsp_fw::PipelinesListInfo, ppl_id);

    /* Constructing ioctl output structure*/
    BigCmdModuleAccessIoctlOutput<dsp_fw::PipelinesListInfo> ioctlOutput(
        dsp_fw::PIPELINE_LIST_INFO_GET, parameterSize);

    /* Performing ioctl */
    bigGetModuleAccessIoctl<dsp_fw::PipelinesListInfo>(ioctlOutput);

    /* Checking returned pipeline count */
    const dsp_fw::PipelinesListInfo &pipelineListInfo = ioctlOutput.getFirmwareParameter();
    if (pipelineListInfo.ppl_count > maxPplCount) {
        throw Exception("Firmware has returned an invalid pipeline count: " +
            std::to_string(pipelineListInfo.ppl_count) +  " max is: " +
            std::to_string(maxPplCount));
    }

    /* Retrieving pipeline entries */
    for (std::size_t i = 0; i < pipelineListInfo.ppl_count; i++) {
        pipelinesIds.push_back(pipelineListInfo.ppl_id[i]);
    }
}

void ModuleHandler::getPipelineProps(uint32_t pipelineId, DSPplProps &props)
{
    /* Constructing ioctl output structure
     * According to the SwAS allocate a single page, i.e. 4096 bytes
     */
    BigCmdModuleAccessIoctlOutput<char> ioctlOutput(
        dsp_fw::PIPELINE_PROPS_GET, maxParameterPayloadSize);

    /* Filling input argument*/
    dsp_fw::PplPropsIn &pplPropsIn =
        reinterpret_cast<dsp_fw::PplPropsIn &>(ioctlOutput.getFirmwareParameter());
    pplPropsIn.ppl_id = pipelineId;

    /* Performing ioctl */
    bigGetModuleAccessIoctl<char>(ioctlOutput);

    /* Getting result as byte array*/
    std::vector<uint8_t> content;
    ioctlOutput.getFirmwareParameterContent(content);

    /* Parsing result into DSPplProps */
    try {
        util::ByteStreamReader reader(content);
        props.fromStream(reader);
    }
    catch (util::ByteStreamReader::Exception &e)
    {
        throw Exception("Cannot parse PplProps content: " + std::string(e.what()));
    }
}

void ModuleHandler::getSchedulersInfo(uint32_t coreId, DSSchedulersInfo &schedulers)
{
    /* Constructing ioctl output structure
    * According to the SwAS allocate a single page, i.e. 4096 bytes
    */
    BigCmdModuleAccessIoctlOutput<char> ioctlOutput(
        dsp_fw::SCHEDULERS_INFO_GET, maxParameterPayloadSize);

    /* Filling input argument*/
    dsp_fw::SchedulersInfoIn &schedInfoIn =
        reinterpret_cast<dsp_fw::SchedulersInfoIn &>(ioctlOutput.getFirmwareParameter());
    schedInfoIn.core_id = coreId;

    /* Performing ioctl */
    bigGetModuleAccessIoctl<char>(ioctlOutput);

    /* Getting result as byte array*/
    std::vector<uint8_t> content;
    ioctlOutput.getFirmwareParameterContent(content);

    /* Parsing result into DSSchedulersInfo */
    try {
        util::ByteStreamReader reader(content);
        schedulers.fromStream(reader);
    }
    catch (util::ByteStreamReader::Exception &e)
    {
        throw Exception("Cannot parse SchedulersInfo content: " + std::string(e.what()));
    }
}

void ModuleHandler::getGatewaysInfo(uint32_t gatewayCount,
    std::vector<dsp_fw::GatewayProps> &gateways)
{
    /* Calculating the memory space required */
    std::size_t parameterSize =
        MEMBER_SIZE(dsp_fw::GatewaysInfo, gateway_count) +
        gatewayCount * MEMBER_SIZE(dsp_fw::GatewaysInfo, gateways);

    /* Constructing ioctl output structure*/
    BigCmdModuleAccessIoctlOutput<dsp_fw::GatewaysInfo> ioctlOutput(
        dsp_fw::GATEWAYS_INFO_GET, parameterSize);

    /* Performing ioctl */
    bigGetModuleAccessIoctl<dsp_fw::GatewaysInfo>(ioctlOutput);

    /* Checking returned gateway count */
    const dsp_fw::GatewaysInfo &gatewaysInfo = ioctlOutput.getFirmwareParameter();
    if (gatewaysInfo.gateway_count > gatewayCount) {
        throw Exception("Firmware has returned an invalid gateway count: " +
            std::to_string(gatewaysInfo.gateway_count) + " max is: " +
            std::to_string(gatewayCount));
    }

    /* Retrieving gateways entries */
    for (std::size_t i = 0; i < gatewaysInfo.gateway_count; i++) {
        gateways.push_back(gatewaysInfo.gateways[i]);
    }
}

}
}
}
