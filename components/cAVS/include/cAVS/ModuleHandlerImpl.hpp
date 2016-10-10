/*
 * Copyright (c) 2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

private:
    friend class ModuleHandler;

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
