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

#pragma once

#include "cAVS/DspFw/ModuleType.hpp"
#include "cAVS/DspFw/ModuleInstance.hpp"
#include "cAVS/DspFw/FwConfig.hpp"
#include "cAVS/DspFw/HwConfig.hpp"
#include "cAVS/DspFw/Pipeline.hpp"
#include "cAVS/DspFw/Gateway.hpp"
#include "cAVS/DspFw/GlobalPerfData.hpp"
#include "cAVS/DspFw/Scheduler.hpp"
#include "DspFw/Common.hpp"
#include "cAVS/DspFw/Infrastructure.hpp"
#include <stdexcept>
#include <vector>

namespace debug_agent
{
namespace cavs
{

/** This abstract class exposes the FW module API */
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
    std::vector<dsp_fw::ModuleEntry> getModulesEntries(uint32_t moduleCount);

    /** @return the firmware configuration */
    dsp_fw::FwConfig getFwConfig();

    /** @return the hardware configuration */
    dsp_fw::HwConfig getHwConfig();

    /** @return the pipeline identifier list */
    std::vector<dsp_fw::PipeLineIdType> getPipelineIdList(uint32_t maxPplCount);

    /** @return the properties of one pipeline */
    dsp_fw::PplProps getPipelineProps(dsp_fw::PipeLineIdType pipelineId);

    /** @return the schedulers of one core */
    dsp_fw::SchedulersInfo getSchedulersInfo(dsp_fw::CoreId coreId);

    /** @return the gateways */
    std::vector<dsp_fw::GatewayProps> getGatewaysInfo(uint32_t gatewayCount);

    /** @return the performance items */
    std::vector<dsp_fw::PerfDataItem> getPerfItems(uint32_t itemCount);

    /** @return the properties of one module instance */
    dsp_fw::ModuleInstanceProps getModuleInstanceProps(uint16_t moduleId, uint16_t instanceId);

    /** set module parameter */
    void setModuleParameter(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                            const util::Buffer &parameterPayload);

    /** @return module parameter */
    util::Buffer getModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                    dsp_fw::ParameterId parameterId,
                                    size_t parameterSize = maxParameterPayloadSize);

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
    /** Perform a "config get" command
     *
     * This method should be implemented using driver specificities
     *
     * @param[in] moduleId the module type id
     * @param[in] instanceId the module instance id
     * @param[in] parameterId the parameter id
     * @param[in] parameterSize the parameter's size
     *
     * @returns the parameter payload.
     * @throw ModuleHandler::Exception
     */
    virtual util::Buffer configGet(uint16_t moduleId, uint16_t instanceId,
                                   dsp_fw::ParameterId parameterId, size_t parameterSize) = 0;

    /** Perform a "config set" command
     *
     * This method should be implemented using driver specificities
     *
     * @param[in] moduleId the module type id
     * @param[in] instanceId the module instance id
     * @param[in] parameterId the parameter id
     * @param[in] parameterPayload the parameter payload to set as value
     *
     * @throw ModuleHandler::Exception
     */
    virtual void configSet(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                           const util::Buffer &parameterPayload) = 0;

    /** Get module parameter value as a template type
     * @tparam FirmwareParameterType The type of the retrieved parameter value
     */
    template <typename FirmwareParameterType>
    void getFwParameterValue(uint16_t moduleId, uint16_t instanceId,
                             dsp_fw::ParameterId moduleParamId, std::size_t fwParameterSize,
                             FirmwareParameterType &result);

    /** Get a tlv list from a module parameter value */
    template <typename TlvResponseHandlerInterface>
    TlvResponseHandlerInterface readTlvParameters(dsp_fw::BaseFwParams parameterId);

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
