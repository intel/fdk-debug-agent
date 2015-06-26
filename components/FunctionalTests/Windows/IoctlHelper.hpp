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

#include "cAVS/Windows/MockedDevice.hpp"

/** Helper to fill ioctl test vector */
class IoctlHelper final
{
public:
    using ModuleUID = uint32_t[4];

    /** two sample modules UUID */
    static const ModuleUID Module0UUID;
    static const ModuleUID Module1UUID;

    /** Add a module entry command to the mocked device.
      *
      * Two modules are added : module_0 and module_1, associated to Module0UUID and Module1UUID
      */
    static void addModuleEntryCommand(debug_agent::cavs::windows::MockedDevice &device);
private:
    IoctlHelper();
};