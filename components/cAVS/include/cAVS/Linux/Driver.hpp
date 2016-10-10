/*
 * Copyright (c) 2015-2016, Intel Corporation
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
#include "cAVS/Linux/Device.hpp"
#include "cAVS/Linux/ControlDevice.hpp"
#include "cAVS/DspFw/FwConfig.hpp"
#include "cAVS/DspFw/HwConfig.hpp"
#include "cAVS/Linux/ModuleHandlerImpl.hpp"
#include "cAVS/Linux/Logger.hpp"
#include "cAVS/Linux/Perf.hpp"
#include "cAVS/Linux/Prober.hpp"
#include "Util/AssertAlways.hpp"
#include <utility>

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
    Driver(std::unique_ptr<Device> device, std::unique_ptr<ControlDevice> controlDevice,
           std::unique_ptr<CompressDeviceFactory> compressDeviceFactory)
        : mDevice(std::move(device)), mControlDevice(std::move(controlDevice)),
          mCompressDeviceFactory(std::move(compressDeviceFactory)),
          mLogger(*mDevice, *mControlDevice, *mCompressDeviceFactory),
          mProber(*mControlDevice, *mCompressDeviceFactory),
          mModuleHandler(std::make_unique<ModuleHandlerImpl>(*mDevice)),
          mPerf(*mDevice, mModuleHandler)
    {
        ASSERT_ALWAYS(mCompressDeviceFactory != nullptr);
    }

    cavs::Logger &getLogger() override { return mLogger; }
    cavs::ModuleHandler &getModuleHandler() override { return mModuleHandler; }
    cavs::Prober &getProber() override { return mProber; }
    cavs::Perf &getPerf() override { return mPerf; }

private:
    std::unique_ptr<Device> mDevice;
    std::unique_ptr<ControlDevice> mControlDevice;
    std::unique_ptr<CompressDeviceFactory> mCompressDeviceFactory;
    Logger mLogger;
    Prober mProber;
    ModuleHandler mModuleHandler;
    Perf mPerf;
};
}
}
}
