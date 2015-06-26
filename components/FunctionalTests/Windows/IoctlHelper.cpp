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

#include "Windows/IoctlHelper.hpp"
#include "cAVS/FirmwareTypes.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/IoCtlStructureHelpers.hpp"

using namespace debug_agent::cavs;
using namespace debug_agent::cavs::windows;

template <typename T>
static void copyTypeMemoryContent(T &dst, const T &src)
{
    std::memcpy(&dst, &src, sizeof(T));
}

const IoctlHelper::ModuleUID IoctlHelper::Module0UUID = { 1, 2, 3, 4 };
const IoctlHelper::ModuleUID IoctlHelper::Module1UUID = { 11, 12, 13, 14 };

void setModuleEntry(dsp_fw::ModuleEntry &entry, const std::string &name,
    const IoctlHelper::ModuleUID &uuid)
{
    /* Setting name */
    assert(name.size() <= sizeof(entry.name));
    for (std::size_t i = 0; i < sizeof(entry.name); i++) {
        if (i < name.size()) {
            entry.name[i] = name[i];
        }
        else {
            /* Filling buffer with 0 after name end */
            entry.name[i] = 0;
        }
    }

    /* Setting GUID*/
    copyTypeMemoryContent<IoctlHelper::ModuleUID>(entry.uuid, uuid);
}

void IoctlHelper::addModuleEntryCommand(MockedDevice &device)
{
    std::size_t moduleInfoSize = ModulesInfoHelper::getAllocationSize();

    /* Expected output buffer*/
    BigCmdIoctlOutput<dsp_fw::ModulesInfo>
        expectedOutput(dsp_fw::MODULES_INFO_GET, moduleInfoSize);

    /* Returned output buffer*/
    BigCmdIoctlOutput<dsp_fw::ModulesInfo>
        returnedOutput(dsp_fw::MODULES_INFO_GET, moduleInfoSize);

    /* Result codes */
    returnedOutput.getCmdBody().Status = STATUS_SUCCESS;
    returnedOutput.getModuleParameterAccess().FwStatus =
        dsp_fw::Message::IxcStatus::ADSP_IPC_SUCCESS;

    /* Settings module entry content */
    dsp_fw::ModulesInfo &modulesInfo = returnedOutput.getFirmwareParameter();
    modulesInfo.module_count = 2; /* Returning 2 modules */

    /* Module #1 */
    setModuleEntry(modulesInfo.module_info[0], "module_0", Module0UUID);

    /* Module #2 */
    setModuleEntry(modulesInfo.module_info[1], "module_1", Module1UUID);

    /* Filling expected input buffer */
    windows::TypedBuffer<windows::driver::Intc_App_Cmd_Header> expectedInput;
    expectedInput->FeatureID = static_cast<ULONG>(windows::driver::FEATURE_MODULE_PARAMETER_ACCESS);
    expectedInput->ParameterID = 0; /* only one parameter id for this feature */
    expectedInput->DataSize = static_cast<ULONG>(expectedOutput.getBuffer().getSize());

    /* Adding entry */
    device.addIoctlEntry(IOCTL_CMD_APP_TO_AUDIODSP_BIG_GET, &expectedInput,
        &expectedOutput.getBuffer(), &returnedOutput.getBuffer(), true);
}
