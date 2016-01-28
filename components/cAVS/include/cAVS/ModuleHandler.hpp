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
#include "DspFw/Common.hpp"
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

    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
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
    virtual void getPipelineIdList(uint32_t maxPplCount,
                                   std::vector<dsp_fw::PipeLineIdType> &pipelinesIds) = 0;

    /** @return the properties of one pipeline */
    virtual void getPipelineProps(dsp_fw::PipeLineIdType pipelineId, dsp_fw::PplProps &props) = 0;

    /** @return the schedulers of one core */
    virtual void getSchedulersInfo(dsp_fw::CoreId coreId, dsp_fw::SchedulersInfo &schedulers) = 0;

    /** @return the gateways */
    virtual void getGatewaysInfo(uint32_t gatewayCount,
                                 std::vector<dsp_fw::GatewayProps> &gateways) = 0;

    /** @return the properties of one module instance */
    virtual void getModuleInstanceProps(uint16_t moduleId, uint16_t instanceId,
                                        dsp_fw::ModuleInstanceProps &props) = 0;

    /** set module parameter */
    virtual void setModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                    dsp_fw::ParameterId parameterId,
                                    const util::Buffer &parameterPayload) = 0;

    /** @return module parameter */
    virtual void getModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                    dsp_fw::ParameterId parameterId,
                                    util::Buffer &parameterPayload) = 0;

    /** The base firmware has several module like components in it.
     * To address them, the ParameterId is splited in a type and an instance part.
     * @{
     */

    /** @return the parameter id addressing the requested core parameters. */
    static dsp_fw::ParameterId getExtendedParameterId(dsp_fw::BaseFwParams parameterTypeId,
                                                      dsp_fw::CoreId coreId)
    {
        return getExtendedParameterId(parameterTypeId, coreId.getValue());
    }

    /** @return the parameter id addressing the requested pipeline parameters. */
    static dsp_fw::ParameterId getExtendedParameterId(dsp_fw::BaseFwParams parameterTypeId,
                                                      dsp_fw::PipeLineIdType pipelineId)
    {
        return getExtendedParameterId(parameterTypeId, pipelineId.getValue());
    }
    /** @} */

private:
    /** @return extended parameter id that contains the targeted module part id */
    static dsp_fw::ParameterId getExtendedParameterId(dsp_fw::BaseFwParams parameterTypeId,
                                                      uint32_t parameterInstanceId)
    {
        uint32_t parameterTypeIdAsInt = static_cast<uint32_t>(parameterTypeId);
        assert(parameterTypeIdAsInt < (1 << 8));
        assert(parameterInstanceId < (1 << 24));

        return dsp_fw::ParameterId{(parameterTypeIdAsInt & 0xFF) | (parameterInstanceId << 8)};
    }

    ModuleHandler(const ModuleHandler &) = delete;
    ModuleHandler &operator=(const ModuleHandler &) = delete;
};
}
}
