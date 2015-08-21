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
#include <vector>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

template <typename FirmwareParameterType>
void ModuleHandler::bigGetModuleAccessIoctl(
    BigCmdModuleAccessIoctlOutput<FirmwareParameterType> &output)
{
    /* Creating ioctl input structure */
    TypedBuffer<driver::Intc_App_Cmd_Header> ioctlInput;
    ioctlInput->FeatureID = static_cast<ULONG>(driver::FEATURE_MODULE_PARAMETER_ACCESS);
    ioctlInput->ParameterID = moduleParameterAccessParameterId;
    ioctlInput->DataSize = static_cast<ULONG>(output.getBuffer().getSize());

    /* Performing the io ctl */
    try
    {
        mDevice.ioControl(IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET, &ioctlInput, &output.getBuffer());
    }
    catch (Device::Exception &e)
    {
        throw Exception("Device returns an exception: " + std::string(e.what()));
    }

    /* Checking driver status */
    if (!NT_SUCCESS(output.getCmdBody().Status))
    {
        throw Exception("Driver returns invalid status: " +
            std::to_string(static_cast<uint32_t>(output.getCmdBody().Status)));
    }

    /* Checking firwmare status */
    if (output.getModuleParameterAccess().FwStatus !=
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS) {
        throw Exception("Firmware returns invalid status: " +
            std::to_string(static_cast<uint32_t>(output.getModuleParameterAccess().FwStatus)));
    }
}

void ModuleHandler::getAdspProperties(dsp_fw::AdspProperties &properties)
{
    /* Constructing ioctl output structure*/
    BigCmdModuleAccessIoctlOutput<dsp_fw::AdspProperties> ioctlOutput(
        dsp_fw::ADSP_PROPERTIES, sizeof(dsp_fw::AdspProperties));

    /* Performing ioctl */
    bigGetModuleAccessIoctl<dsp_fw::AdspProperties>(ioctlOutput);

    /* Retrieving properties */
    properties = ioctlOutput.getFirmwareParameter();
}

void ModuleHandler::getModulesEntries(std::vector<ModuleEntry> &modulesEntries)
{
    std::size_t moduleInfoSize = ModulesInfoHelper::getAllocationSize();

    /* Constructing ioctl output structure */
    BigCmdModuleAccessIoctlOutput<dsp_fw::ModulesInfo>
        ioctlOutput(dsp_fw::MODULES_INFO_GET, moduleInfoSize);

    /* Performing ioctl */
    bigGetModuleAccessIoctl<dsp_fw::ModulesInfo>(ioctlOutput);

    /* Checking returned module count */
    const dsp_fw::ModulesInfo &modulesInfo = ioctlOutput.getFirmwareParameter();
    if (modulesInfo.module_count > dsp_fw::MaxModuleCount) {
        throw Exception("Firmware has returned an invalid module count: " +
            std::to_string(modulesInfo.module_count));
    }

    /* Retrieving module entries */
    for (std::size_t i = 0; i < modulesInfo.module_count; i++) {
        modulesEntries.push_back(modulesInfo.module_info[i]);
    }
}

}
}
}
