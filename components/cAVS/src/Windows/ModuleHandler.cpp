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
#include "cAVS/Windows/ModuleHandler.hpp"
#include "cAVS/Windows/WindowsTypes.hpp"
#include "cAVS/Windows/IoctlHelpers.hpp"
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

util::Buffer ModuleHandler::bigCmdModuleAccessIoctl(bool isGet, uint16_t moduleId,
                                                    uint16_t instanceId,
                                                    dsp_fw::ParameterId moduleParamId,
                                                    const util::Buffer &suppliedOutputBuffer)
{
    /* Creating the body payload using the IoctlFwModuleParam type */
    driver::IoctlFwModuleParam moduleParam(moduleId, instanceId, moduleParamId,
                                           static_cast<uint32_t>(suppliedOutputBuffer.size()));

    util::ByteStreamWriter bodyPayloadWriter;
    bodyPayloadWriter.write(moduleParam);
    bodyPayloadWriter.writeRawBuffer(suppliedOutputBuffer);

    /* Creating BigGet/Set ioctl buffers */
    util::Buffer headerBuffer;
    util::Buffer bodyBuffer;
    std::tie(headerBuffer, bodyBuffer) = IoctlHelpers::toBigCmdBuffers(
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
        std::tie(driverStatus, bodyPayloadBuffer) = IoctlHelpers::fromBigCmdBodyBuffer(bodyBuffer);

        /* Checking driver status */
        if (!NT_SUCCESS(driverStatus)) {
            throw Exception("Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(driverStatus)));
        }

        /* Reading driver IoctlFwModuleParam structure from body payload */
        util::ByteStreamReader reader(bodyPayloadBuffer);
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
util::Buffer ModuleHandler::configGet(uint16_t moduleId, uint16_t instanceId,
                                      dsp_fw::ParameterId parameterId, size_t parameterSize)
{
    const util::Buffer suppliedParameterPayload(parameterSize, 0xff);
    return bigCmdModuleAccessIoctl(true, moduleId, instanceId, parameterId,
                                   suppliedParameterPayload);
}

/** @return module parameter */
void ModuleHandler::configSet(uint16_t moduleId, uint16_t instanceId,
                              dsp_fw::ParameterId parameterId,
                              const util::Buffer &suppliedParameterPayload)
{
    bigCmdModuleAccessIoctl(false, moduleId, instanceId, parameterId, suppliedParameterPayload);
}
}
}
}
