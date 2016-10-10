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
#include "cAVS/Windows/Prober.hpp"
#include <Util/AssertAlways.hpp>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

Prober::~Prober()
{
    mStateMachine.stopNoThrow();
}

void Prober::setState(bool active)
{
    try {
        mStateMachine.setState(active);
    } catch (ProberStateMachine::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}

bool Prober::isActive()
{
    try {
        return mStateMachine.isActive();
    } catch (ProberStateMachine::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}

std::unique_ptr<util::Buffer> Prober::dequeueExtractionBlock(ProbeId probeIndex)
{
    try {
        return mBackend.dequeueExtractionBlock(probeIndex);
    } catch (ProberBackend::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}

bool Prober::enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer)
{
    try {
        return mBackend.enqueueInjectionBlock(probeIndex, buffer);
    } catch (ProberBackend::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}

std::size_t Prober::getMaxProbeCount() const
{
    return mBackend.getMaxProbeCount();
}

void Prober::setProbesConfig(const SessionProbes &probes,
                             const InjectionSampleByteSizes &injectionSampleByteSizes)
{
    try {
        mBackend.setSessionProbes(probes, injectionSampleByteSizes);
    } catch (ProberBackend::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}
}
}
}
