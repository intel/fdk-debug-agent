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

#include "cAVS/ModuleHandler.hpp"
#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "tlv/TlvResponseHandlerInterface.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Windows module handler implementation */
class ModuleHandler : public cavs::ModuleHandler
{
public:
    ModuleHandler(Device &device) : mDevice(device) {}

    util::Buffer configGet(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                           size_t parameterSize) override;

    void configSet(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                   const util::Buffer &parameterPayload) override;

private:
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
