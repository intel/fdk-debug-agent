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
#include "cAVS/DspFw/FwConfig.hpp"
#include "cAVS/DspFw/HwConfig.hpp"
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
        virtual void getFwConfig(dsp_fw::FwConfig &fwConfig) override {}
        virtual void getHwConfig(dsp_fw::HwConfig &hwConfig) override {}
        virtual void getModulesEntries(uint32_t moduleCount,
            std::vector<dsp_fw::ModuleEntry> &modulesEntries) override {}
        virtual void getPipelineIdList(uint32_t maxPplCount,
            std::vector<uint32_t> &pipelinesIds) override {}
        virtual void getPipelineProps(uint32_t pipelineId, dsp_fw::DSPplProps &props) override {}
        virtual void getSchedulersInfo(uint32_t coreId,
            dsp_fw::DSSchedulersInfo &schedulers) override {}
        virtual void getGatewaysInfo(uint32_t gatewayCount,
            std::vector<dsp_fw::GatewayProps> &gateways) override {}
        virtual void getModuleInstanceProps(uint16_t moduleId, uint16_t instanceId,
            dsp_fw::DSModuleInstanceProps &props) override {}
        virtual void setModuleParameter(uint16_t moduleId, uint16_t instanceId,
            uint32_t parameterId, const std::vector<uint8_t> &parameterPayload) override {}
        virtual void getModuleParameter(uint16_t moduleId, uint16_t instanceId,
            uint32_t parameterId, std::vector<uint8_t> &parameterPayload) override {}
    };

    Logger mLogger;
    DummyModuleHandler mModuleHandler;
};

}
}
}
