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

#include "cAVS/Driver.hpp"
#include "cAVS/Linux/Logger.hpp"

namespace debug_agent
{
namespace cavs
{
namespace linuxx /* using 'linuxx' instead of 'linux' because 'linux' is an existing symbol */
{

/**
 * Defines the cavs::Driver for Linux Driver interface.
 */
class Driver final : public cavs::Driver
{
public:
    virtual cavs::Logger &getLogger() override { return mLogger; }
    virtual ModuleHandler &getModuleHandler() override { return mModuleHandler; }

private:
    /* Will be replaced by the true implementation*/
    class DummyModuleHandler : public ModuleHandler
    {
    public:
        virtual void getAdspProperties(dsp_fw::AdspProperties &properties) override {}
        virtual void getModulesEntries(std::vector<dsp_fw::ModuleEntry> &modulesEntries)
            override {}
    };

    Logger mLogger;
    DummyModuleHandler mModuleHandler;
};

}
}
}