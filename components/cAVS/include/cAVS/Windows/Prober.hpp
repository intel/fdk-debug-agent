/*
 * Copyright (c) 2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
