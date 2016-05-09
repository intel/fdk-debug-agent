/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "cAVS/DspFw/Common.hpp"
#include "Util/Buffer.hpp"
#include "Util/Exception.hpp"

#include <cstdint>
#include <cstddef>

namespace debug_agent
{
namespace cavs
{

/** module handler implementation */
class ModuleHandlerImpl
{
public:
    using Exception = util::Exception<ModuleHandlerImpl>;

    virtual ~ModuleHandlerImpl() = default;

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
};
}
}
