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

#include "cAVS/DspFw/ModuleType.hpp"
#include "cAVS/DspFw/ModuleInstance.hpp"
#include "cAVS/DspFw/FwConfig.hpp"
#include "cAVS/DspFw/HwConfig.hpp"
#include "cAVS/DspFw/Pipeline.hpp"
#include "cAVS/DspFw/Gateway.hpp"
#include "cAVS/DspFw/Scheduler.hpp"
#include "cAVS/DspFw/Infrastructure.hpp"
#include <stdexcept>
#include <vector>

namespace debug_agent
{
namespace cavs
{

/** This abstract class exposes firmware module */
class ModuleHandler
{
public:
    /**
     * The size in bytes of the response buffer needed by the FW in order to reply a TLV.
     * The SwAS specifies the output buffer size for TLV shall be 2KB.
     */
    static const size_t cavsTlvBufferSize = 2048;

    /**
     * Max parameter payload size set to one memory page (4096 bytes)
     */
    static const size_t maxParameterPayloadSize = 4 * 1024;

    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    ModuleHandler() = default;
    virtual ~ModuleHandler() {}

    /** @return the firmware module entries */
    virtual void getModulesEntries(uint32_t moduleCount,
        std::vector<dsp_fw::ModuleEntry> &modulesEntries) = 0;

    /** @return the firmware configuration */
    virtual void getFwConfig(dsp_fw::FwConfig &fwConfig) = 0;

    /** @return the hardware configuration */
    virtual void getHwConfig(dsp_fw::HwConfig &hwConfig) = 0;

    /** @return the pipeline identifier list */
    virtual void getPipelineIdList(uint32_t maxPplCount, std::vector<uint32_t> &pipelinesIds) = 0;

    /** @return the properties of one pipeline */
    virtual void getPipelineProps(uint32_t pipelineId, dsp_fw::PplProps &props) = 0;

    /** @return the schedulers of one core */
    virtual void getSchedulersInfo(uint32_t coreId, dsp_fw::SchedulersInfo &schedulers) = 0;

    /** @return the gateways */
    virtual void getGatewaysInfo(uint32_t gatewayCount,
        std::vector<dsp_fw::GatewayProps> &gateways) = 0;

    /** @return the properties of one module instance */
    virtual void getModuleInstanceProps(uint16_t moduleId, uint16_t instanceId,
        dsp_fw::ModuleInstanceProps &props) = 0;

    /** set module parameter */
    virtual void setModuleParameter(uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
        const std::vector<uint8_t> &parameterPayload) = 0;

    /** @return module parameter */
    virtual void getModuleParameter(uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
        std::vector<uint8_t> &parameterPayload) = 0;

    /** @return extended parameter id that contains the targeted module part id */
    static uint32_t getExtendedParameterId(dsp_fw::BaseFwParams parameterTypeId,
        uint32_t parameterInstanceId)
    {
        uint32_t parameterTypeIdAsInt = static_cast<uint32_t>(parameterTypeId);
        assert(parameterTypeIdAsInt < (1 << 8));
        assert(parameterInstanceId < (1 << 24));

        return (parameterTypeIdAsInt & 0xFF) | (parameterInstanceId << 8);
    }

private:
    ModuleHandler(const ModuleHandler &) = delete;
    ModuleHandler &operator=(const ModuleHandler &) = delete;
};

}
}
