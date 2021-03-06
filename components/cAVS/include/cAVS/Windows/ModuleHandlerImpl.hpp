/*
 * Copyright (c) 2015-2016, Intel Corporation
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

#include "cAVS/ModuleHandlerImpl.hpp"
#include "cAVS/Windows/Device.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Windows module handler implementation */
class ModuleHandlerImpl : public cavs::ModuleHandlerImpl
{
public:
    ModuleHandlerImpl(Device &device) : mDevice(device) {}

private:
    util::Buffer configGet(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                           size_t parameterSize) override;

    void configSet(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                   const util::Buffer &parameterPayload) override;

    Device &mDevice;

    /** Performs an ioctl "big get/set" to the base firmware using the feature
     * "module access parameter" to retrieve firmware information: adsp properties, module entries,
     * pipelines...
     *
     * The ioctl "big get/set" allows to retrieve information from the driver. This ioctl supports
     * several kind of information (Wake On voice...), here the "module access parameter" is used
     * to retrieve firmware structures.
     *
     * @param[in] isGet true for BigGet ioctl, false for BigSet ioctl.
     * @param[in] moduleId the target module type id
     * @param[in] instanceId the target module instance id
     * @param[in] moduleParamId the module parameter id
     * @param[in] suppliedOutputBuffer the input parameter payload
     * @param[in] returnedOutputBuffer the output parameter payload
     */
    util::Buffer bigCmdModuleAccessIoctl(bool isGet, uint16_t moduleId, uint16_t instanceId,
                                         dsp_fw::ParameterId moduleParamId,
                                         const util::Buffer &suppliedOutputBuffer);
};
}
}
}
