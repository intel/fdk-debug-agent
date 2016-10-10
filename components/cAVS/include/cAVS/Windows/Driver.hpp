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

#include "cAVS/Driver.hpp"
#include "cAVS/Windows/EventHandle.hpp"
#include "cAVS/Windows/Logger.hpp"
#include "cAVS/Windows/ModuleHandlerImpl.hpp"
#include "cAVS/Windows/Prober.hpp"
#include "cAVS/Windows/Perf.hpp"
#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/WppClientFactory.hpp"
#include "Util/AssertAlways.hpp"
#include <memory>
#include <utility>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/**
 * Defines the cavs::Driver for Windows driver interface.
 */
class Driver final : public cavs::Driver
{
public:
    Driver(std::unique_ptr<Device> device, std::unique_ptr<WppClientFactory> wppClientFactory,
           ProberBackend::EventHandles &eventHandles)
        : mDevice(std::move(device)), mWppClientFactory(std::move(wppClientFactory)),
          mEventHandles(std::move(eventHandles)), mLogger(*mDevice, *mWppClientFactory),
          mModuleHandler(std::make_unique<ModuleHandlerImpl>(*mDevice)),
          mProber(*mDevice, mEventHandles), mPerf(*mDevice)
    {
        ASSERT_ALWAYS(mEventHandles.isValid());
    }

    cavs::Logger &getLogger() override { return mLogger; }
    cavs::ModuleHandler &getModuleHandler() override { return mModuleHandler; }
    cavs::Prober &getProber() override { return mProber; }
    cavs::Perf &getPerf() override { return mPerf; }

private:
    std::unique_ptr<Device> mDevice;
    std::unique_ptr<WppClientFactory> mWppClientFactory;
    ProberBackend::EventHandles mEventHandles;
    Logger mLogger;
    ModuleHandler mModuleHandler;
    Prober mProber;
    Perf mPerf;
};
}
}
}
