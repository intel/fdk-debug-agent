/*
 * Copyright (c) 2015, Intel Corporation
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

#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/WppClientFactory.hpp"
#include "cAVS/Windows/EventHandle.hpp"
#include "cAVS/Windows/Prober.hpp"
#include "cAVS/DriverFactory.hpp"
#include <memory>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This driver factory injects a device and a wpp client factory into the created
 * driver instance */
class DeviceInjectionDriverFactory : public DriverFactory
{
public:
    DeviceInjectionDriverFactory(std::unique_ptr<Device> injectedDevice,
                                 std::unique_ptr<WppClientFactory> injectedWppClientFactory,
                                 ProberBackend::EventHandles &probeEventHandles)
        : mInjectedDevice(std::move(injectedDevice)),
          mInjectedWppClientFactory(std::move(injectedWppClientFactory)),
          mInjectedProbeEventHandles(std::move(probeEventHandles))
    {
    }

    virtual std::unique_ptr<cavs::Driver> newDriver() const override;

private:
    /** newDriver() is a const method because the factory doesn't suppose to be changed when
     * an instance is created.
     * But in the DeviceInjectionDriverFactory case, the following unique pointer will loose its
     * content when newDriver() is called, so declaring it mutable */
    mutable std::unique_ptr<Device> mInjectedDevice;

    /* Same point for this member */
    mutable std::unique_ptr<WppClientFactory> mInjectedWppClientFactory;

    /* Same point for this member */
    mutable ProberBackend::EventHandles mInjectedProbeEventHandles;
};
}
}
}
