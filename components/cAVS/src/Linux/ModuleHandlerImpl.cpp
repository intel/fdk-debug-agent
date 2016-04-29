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
 *******************************************************************************
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
