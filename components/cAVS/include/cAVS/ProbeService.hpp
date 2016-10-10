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
#include "cAVS/Driver.hpp"
#include "Util/Exception.hpp"
#include <mutex>

namespace debug_agent
{
namespace cavs
{
/** This class handles the configuration and activation cycle of the Prober. */
class ProbeService
{
public:
    using Exception = util::Exception<ProbeService>;

    ProbeService(Driver &driver)
        : mDriver(driver), mProbeConfigs(mDriver.getProber().getMaxProbeCount())
    {
    }

    /** Set the state of the probing service.
     * @throw System::Exception
     */
    void setState(bool active);
    /**
     * Get the state of the probing service
     */
    bool isActive();
    void checkProbeId(ProbeId probeId) const;
    /** Set probe configuration. This configuration will be used at next probe service start.
     * @throw System::Exception
     */
    void setConfiguration(ProbeId id, const Prober::ProbeConfig &probe);
    /**
    * @throw System::Exception
    */
    Prober::ProbeConfig getConfiguration(ProbeId id);

    /** @return the sample byte size of each injection probes used to inject silence if
     * underrun happens.
     */
    const Prober::InjectionSampleByteSizes getInjectionSampleByteSizes() const;

private:
    Driver &mDriver;
    Prober::SessionProbes mProbeConfigs;
    mutable std::mutex mProbeConfigMutex;
};
} // cavs
} // debug_agent
