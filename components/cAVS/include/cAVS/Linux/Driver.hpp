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
    cavs::ModuleHandler &getModuleHandler() override { return mModuleHandler; }
    cavs::Prober &getProber() override { return mProber; }

private:
    /* Will be replaced by the true implementation*/
    class DummyModuleHandler : public ModuleHandler
    {
    private:
        util::Buffer configGet(uint16_t moduleId, uint16_t instanceId,
                               dsp_fw::ParameterId parameterId, size_t parameterSize) override
        {
        }

        void configSet(uint16_t moduleId, uint16_t instanceId, dsp_fw::ParameterId parameterId,
                       const util::Buffer &parameterPayload) override
        {
        }
    };

    /* Will be replaced by the true implementation*/
    class DummyProber : public Prober
    {
    public:
        std::size_t getMaxProbeCount() const override { return 0; }

        void setState(State state) override {}

        State getState() override { return State::Idle; }

        void setSessionProbes(const std::vector<ProbeConfig> probes) override {}

        std::vector<ProbeConfig> getSessionProbes() override { return std::vector<ProbeConfig>(); }

        std::unique_ptr<util::Buffer> dequeueExtractionBlock(ProbeId probeIndex) override
        {
            return nullptr;
        }

        bool enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer) override
        {
            return false;
        }
    };

    Logger mLogger;
    DummyModuleHandler mModuleHandler;
    DummyProber mProber;
};
}
}
}
