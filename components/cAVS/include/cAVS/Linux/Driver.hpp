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
namespace linux
{

/**
 * Defines the cavs::Driver for Linux Driver interface.
 */
class Driver final : public cavs::Driver
{
public:
    cavs::Logger &getLogger() override { return mLogger; }
    ModuleHandler &getModuleHandler() override { return mModuleHandler; }

private:
    /* Will be replaced by the true implementation*/
    class DummyModuleHandler : public ModuleHandler
    {
    public:
        void getFwConfig(dsp_fw::FwConfig &fwConfig) override {}
        void getHwConfig(dsp_fw::HwConfig &hwConfig) override {}
        void getModulesEntries(uint32_t moduleCount,
                               std::vector<dsp_fw::ModuleEntry> &modulesEntries) override
        {
        }
        void getPipelineIdList(uint32_t maxPplCount,
                               std::vector<dsp_fw::PipeLineIdType> &pipelinesIds) override
        {
        }
        void getPipelineProps(dsp_fw::PipeLineIdType pipelineId, dsp_fw::PplProps &props) override
        {
        }
        void getSchedulersInfo(dsp_fw::CoreId coreId, dsp_fw::SchedulersInfo &schedulers) override
        {
        }
        void getGatewaysInfo(uint32_t gatewayCount,
                             std::vector<dsp_fw::GatewayProps> &gateways) override
        {
        }
        void getModuleInstanceProps(uint16_t moduleId, uint16_t instanceId,
                                    dsp_fw::ModuleInstanceProps &props) override
        {
        }
        void setModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                dsp_fw::ParameterId parameterId,
                                const std::vector<uint8_t> &parameterPayload) override
        {
        }
        void getModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                dsp_fw::ParameterId parameterId,
                                std::vector<uint8_t> &parameterPayload) override
        {
        }
    };

    Logger mLogger;
    DummyModuleHandler mModuleHandler;
};
}
}
}
