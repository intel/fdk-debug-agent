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

#pragma once

#include "cAVS/ModuleHandler.hpp"
#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/IoCtlStructureHelpers.hpp"
#include "tlv/TlvResponseHandlerInterface.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Windows module handler implementation */
class ModuleHandler : public cavs::ModuleHandler
{
public:
    ModuleHandler(Device &device) : mDevice(device) {}

    virtual void getModulesEntries(uint32_t moduleCount,
        std::vector<ModuleEntry> &modulesEntries) override;
    virtual void getFwConfig(FwConfig &fwConfig) override;
    virtual void getHwConfig(HwConfig &hwConfig) override;
    virtual void getPipelineIdList(uint32_t maxPplCount,
        std::vector<uint32_t> &pipelinesIds) override;
    virtual void getPipelineProps(uint32_t pipelineId, DSPplProps &props) override;
    virtual void getSchedulersInfo(uint32_t coreId, DSSchedulersInfo &schedulers) override;
    virtual void getGatewaysInfo(uint32_t gatewayCount,
        std::vector<dsp_fw::GatewayProps> &gateways) override;
    virtual void getModuleInstanceProps(uint16_t moduleId, uint16_t instanceId,
        DSModuleInstanceProps &props) override;
    virtual void setModuleParameter(uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
        const std::vector<uint8_t> &parameterPayload) override;
    virtual void getModuleParameter(uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
        std::vector<uint8_t> &parameterPayload) override;

private:
    /** The module parameter access feature has only one parameter, therefore its index is 0 */
    static const uint32_t moduleParameterAccessParameterId = 0;

    template<typename TlvResponseHandlerInterface>
    void readTlvParameters(TlvResponseHandlerInterface &responseHandler,
                           dsp_fw::BaseFwParams parameterId);

    Device &mDevice;

    /** Performs an ioctl "big get/set" to the base firmware using the feature
     * "module access parameter" to retrieve firmware information: adsp properties, module entries,
     * pipelines...
     *
     * The ioctl "big get/set" allows to retrieve information from the driver. This ioctl supports
     * several kind of information (Wake On voice...), here the "module access parameter" is used
     * to retrieve firmware structures.
     *
     * @tparam FirmwareType the firmware type, according to the baseFwParam parameter
     *                      For instance: AdspProperties, ModulesInfo...
     * @param[in, out] output A structure that will receive ioctl result.
     */
    template<typename FirmwareParameterType>
    void bigModuleAccessIoctl(bool isGet,
        BigCmdModuleAccessIoctlOutput<FirmwareParameterType> &output);

    /** Helper that performs large GET ioctl */
    template<typename FirmwareParameterType>
    void bigGetModuleAccessIoctl(BigCmdModuleAccessIoctlOutput<FirmwareParameterType> &output)
    {
        bigModuleAccessIoctl<FirmwareParameterType>(true, output);
    }

    /** Helper that performs large SET ioctl */
    template<typename FirmwareParameterType>
    void bigSetModuleAccessIoctl(BigCmdModuleAccessIoctlOutput<FirmwareParameterType> &output)
    {
        bigModuleAccessIoctl<FirmwareParameterType>(false, output);
    }
};

}
}
}
