/*
 * Copyright (c) 2015, Intel Corporation
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
#include "cAVS/Windows/ModuleHandlerImpl.hpp"
#include "cAVS/Windows/WindowsTypes.hpp"
#include "cAVS/Windows/IoctlHelpers.hpp"
#include "cAVS/DspFw/Infrastructure.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Tlv/TlvUnpack.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

util::Buffer ModuleHandlerImpl::bigCmdModuleAccessIoctl(bool isGet, uint16_t moduleId,
                                                        uint16_t instanceId,
                                                        dsp_fw::ParameterId moduleParamId,
                                                        const util::Buffer &suppliedOutputBuffer)
{
    /* Creating the body payload using the IoctlFwModuleParam type */
    driver::IoctlFwModuleParam moduleParam(moduleId, instanceId, moduleParamId,
                                           static_cast<uint32_t>(suppliedOutputBuffer.size()));

    util::MemoryByteStreamWriter bodyPayloadWriter;
    bodyPayloadWriter.write(moduleParam);
    bodyPayloadWriter.writeRawBuffer(suppliedOutputBuffer);

    /* Creating BigGet/Set ioctl buffers */
    util::Buffer headerBuffer;
    util::Buffer bodyBuffer;
    std::tie(headerBuffer, bodyBuffer) = ioctl_helpers::toBigCmdBuffers(
        static_cast<uint32_t>(driver::IOCTL_FEATURE::FEATURE_FW_MODULE_PARAM),
        driver::moduleParameterAccessCommandParameterId, bodyPayloadWriter.getBuffer());

    /* Performing the io ctl */
    try {
        mDevice.ioControl(isGet ? driver::IoCtlType::BigGet : driver::IoCtlType::BigSet,
                          &headerBuffer, &bodyBuffer);
    } catch (Device::Exception &e) {
        throw Exception("Device returns an exception: " + std::string(e.what()));
    }

    /* Reading the result */
    try {
        NTSTATUS driverStatus;
        util::Buffer bodyPayloadBuffer;

        /* Parsing returned output buffer */
        std::tie(driverStatus, bodyPayloadBuffer) = ioctl_helpers::fromBigCmdBodyBuffer(bodyBuffer);

        /* Checking driver status */
        if (!NT_SUCCESS(driverStatus)) {
            throw Exception("Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(driverStatus)));
        }

        /* Reading driver IoctlFwModuleParam structure from body payload */
        util::MemoryByteStreamReader reader(bodyPayloadBuffer);
        reader.read(moduleParam);

        /* Checking firwmare status */
        if (moduleParam.fw_status != static_cast<uint32_t>(dsp_fw::IxcStatus::ADSP_IPC_SUCCESS)) {
            throw Exception("Firmware returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(moduleParam.fw_status)));
        }

        // return the rest (i.e. the "real" payload) of the reader buffer
        return {begin(reader.getBuffer()) + reader.getPointerOffset(), end(reader.getBuffer())};
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Can not decode returned ioctl output buffer: " + std::string(e.what()));
    }
}

/** set module parameter */
util::Buffer ModuleHandlerImpl::configGet(uint16_t moduleId, uint16_t instanceId,
                                          dsp_fw::ParameterId parameterId, size_t parameterSize)
{
    const util::Buffer suppliedParameterPayload(parameterSize, 0xff);
    return bigCmdModuleAccessIoctl(true, moduleId, instanceId, parameterId,
                                   suppliedParameterPayload);
}

/** @return module parameter */
void ModuleHandlerImpl::configSet(uint16_t moduleId, uint16_t instanceId,
                                  dsp_fw::ParameterId parameterId,
                                  const util::Buffer &suppliedParameterPayload)
{
    bigCmdModuleAccessIoctl(false, moduleId, instanceId, parameterId, suppliedParameterPayload);
}
}
}
}
