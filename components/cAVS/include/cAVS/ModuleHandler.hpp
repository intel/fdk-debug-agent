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

#include "cAVS/FirmwareTypes.hpp"
#include "cAVS/FwConfig.hpp"
#include "cAVS/HwConfig.hpp"
#include <stdexcept>
#include <vector>

namespace debug_agent
{
namespace cavs
{

/** This abstract class exposes firmware module */
class ModuleHandler
{
public:
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };


    ModuleHandler() = default;
    virtual ~ModuleHandler() {}

    /** @return the firmware module entries */
    virtual void getModulesEntries(std::vector<ModuleEntry> &modulesEntries) = 0;

    /** @return the firmware configuration */
    virtual void getFwConfig(FwConfig &fwConfig) = 0;

    /** @return the hardware configuration */
    virtual void getHwConfig(HwConfig &hwConfig) = 0;

private:
    ModuleHandler(const ModuleHandler &) = delete;
    ModuleHandler &operator=(const ModuleHandler &) = delete;
};

}
}
