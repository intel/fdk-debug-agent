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
#include "cAVS/Linux/ModuleHandlerImpl.hpp"
#include "cAVS/Linux/DriverTypes.hpp"
#include "cAVS/Linux/CorePower.hpp"
#include "cAVS/DspFw/Common.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/Buffer.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

namespace debug_agent
{
namespace cavs
{
namespace linux
{
util::Buffer ModuleHandlerImpl::configGet(uint16_t moduleId, uint16_t instanceId,
                                          dsp_fw::ParameterId parameterId, size_t parameterSize)
{
    AutoPreventFromSleeping<Exception> autoPower(mCorePower);
    /* Creating the header and body payload using the LargeConfigAccess type */
    driver::LargeConfigAccess configAccess(driver::LargeConfigAccess::CmdType::Get, moduleId,
                                           instanceId, parameterId.getValue(), parameterSize);

    /* Creating debugfs command buffers. */
    util::MemoryByteStreamWriter messageWriter;
    messageWriter.write(configAccess);

    util::Buffer receivedMessage;
    receivedMessage.resize(maxParameterPayloadSize);

    try {
        mDevice.commandRead(driver::setGetCtrl, messageWriter.getBuffer(), receivedMessage);
    } catch (const Device::Exception &e) {
        throw Exception("Get module parameter failed to read command debugfs in file: " +
                        std::string(driver::setGetCtrl) + ", Device returns an exception: " +
                        std::string(e.what()));
    }

    /* Reading the answer using the header of the corresponding replied debugfs command. */
    util::MemoryByteStreamReader messageReader(receivedMessage);
    messageReader.read(configAccess);
    const auto &payloadBegin = messageReader.getBuffer().begin() + messageReader.getPointerOffset();
    return {payloadBegin, payloadBegin + configAccess.getReplyPayloadSize()};
}

void ModuleHandlerImpl::configSet(uint16_t moduleId, uint16_t instanceId,
                                  dsp_fw::ParameterId parameterId,
                                  const util::Buffer &parameterPayload)
{
    AutoPreventFromSleeping<Exception> autoPower(mCorePower);
    /* Creating the header and body payload using the Large or Module ConfigAccess type */
    util::MemoryByteStreamWriter messageWriter;
    if (parameterId.getValue() == dsp_fw::BaseModuleParams::MOD_INST_ENABLE) {
        driver::ModuleConfigAccess configAccess(driver::ModuleConfigAccess::CmdType::Set, moduleId,
                                                instanceId, parameterId.getValue());
        messageWriter.write(configAccess);
    } else {
        driver::LargeConfigAccess configAccess(driver::LargeConfigAccess::CmdType::Set, moduleId,
                                               instanceId, parameterId.getValue(),
                                               parameterPayload.size(), parameterPayload);
        messageWriter.write(configAccess);
    }
    try {
        mDevice.commandWrite(driver::setGetCtrl, messageWriter.getBuffer());
    } catch (const Device::Exception &e) {
        throw Exception("Get module parameter failed to write command debugfs in file: " +
                        std::string(driver::setGetCtrl) + ", Device returns an exception: " +
                        std::string(e.what()));
    }
}
}
}
}
