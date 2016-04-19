/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "cAVS/Prober.hpp"
#include "cAVS/Windows/ProberBackend.hpp"
#include "cAVS/Windows/ProberStateMachine.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class pilots the driver probe service
 *
 * The client changes only a simple boolean state (active, stopped). Changing this state
 * leads to set driver probe service state according the state machine specified in the SwAS.
 * Note: Driver probe service states are (idle, owned, allocated, active)
 *
 * By choice, the high level state (active, stopped) is not stored in this class, but retrieved
 * from driver. This helps to avoid divergent states.
 *
 * The public API of this class is thread safe.
 */
class Prober final : public cavs::Prober
{
public:
    Prober(Device &device, const ProberBackend::EventHandles &eventHandles)
        : mBackend(device, eventHandles), mStateMachine(mBackend)
    {
    }
    ~Prober() override;

    std::size_t getMaxProbeCount() const override;

    void setState(bool active) override;

    bool isActive() override;

    void setProbesConfig(const SessionProbes &probes,
                         const InjectionSampleByteSizes &injectionSampleByteSizes) override;

    std::unique_ptr<util::Buffer> dequeueExtractionBlock(ProbeId probeIndex) override;

    bool enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer) override;

private:
    ProberBackend mBackend;
    ProberStateMachine mStateMachine;
};
}
}
}
