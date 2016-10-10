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

#include "cAVS/Linux/CompressDeviceFactory.hpp"
#include "cAVS/Linux/MockedCompressDevice.hpp"
#include <vector>
#include <cassert>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/* This factory creates real compress device */
class MockedCompressDeviceFactory : public CompressDeviceFactory
{
public:
    MockedCompressDeviceFactory() {}

    ~MockedCompressDeviceFactory()
    {
        std::cout << "Clearing" << std::endl;
        mMockedDevices.clear();
    }

    std::unique_ptr<CompressDevice> newCompressDevice(
        const compress::DeviceInfo &info) const override
    {
        // Do we request an extraction device?
        if (mMockedProbeExtractDevice != nullptr &&
            info.name() == mMockedProbeExtractDevice->getInfo().name()) {
            return std::move(mMockedProbeExtractDevice);
        }
        // Do we request an injection device?
        for (auto &injectionDevice : mMockedProbeInjectDevices) {
            if (info.name() == injectionDevice->getInfo().name()) {
                return std::move(injectionDevice);
            }
        }
        // so we must be requesting a logger device
        std::unique_ptr<CompressDevice> compressDevice(std::move(mMockedDevices.front()));
        if (compressDevice == nullptr) {
            throw CompressDeviceFactory::Exception("");
        }
        assert(compressDevice->getName() == info.name());
        mMockedDevices.erase(mMockedDevices.begin());
        return std::move(compressDevice);
    }

    const compress::LoggersInfo getLoggerDeviceInfoList() const override
    {
        unsigned int coreId = 0;
        compress::LoggersInfo loggersInfo{};
        for (const auto &mockedCompressDevice : mMockedDevices) {
            compress::DeviceInfo info = mockedCompressDevice->getInfo();
            compress::LoggerInfo loggerInfo{info.cardId(), info.deviceId(), coreId++};
            loggersInfo.push_back(loggerInfo);
        }
        return loggersInfo;
    }

    void addMockedDevice(std::unique_ptr<MockedCompressDevice> mockedDevice)
    {
        mMockedDevices.push_back(std::move(mockedDevice));
    }

    void setMockedProbeExtractDevice(std::unique_ptr<MockedCompressDevice> mockedDevice)
    {
        mMockedProbeExtractDevice = std::move(mockedDevice);
    }
    void addMockedProbeInjectDevice(std::unique_ptr<MockedCompressDevice> mockedDevice)
    {
        mMockedProbeInjectDevices.push_back(std::move(mockedDevice));
    }

    const compress::InjectionProbesInfo getInjectionProbeDeviceInfoList() const override
    {
        std::size_t probeIndex = 0;
        compress::InjectionProbesInfo injectionProbesInfo{};
        for (const auto &mockedProbeInjectDevice : mMockedProbeInjectDevices) {
            compress::DeviceInfo info = mockedProbeInjectDevice->getInfo();
            injectionProbesInfo.push_back({info.cardId(), info.deviceId(), probeIndex++});
        }
        return injectionProbesInfo;
    }

    const compress::ExtractionProbeInfo getExtractionProbeDeviceInfo() const override
    {
        assert(mMockedProbeExtractDevice != nullptr);
        return {mMockedProbeExtractDevice->getInfo().cardId(),
                mMockedProbeExtractDevice->getInfo().deviceId()};
    }

private:
    mutable std::vector<std::unique_ptr<MockedCompressDevice>> mMockedDevices;
    mutable std::unique_ptr<MockedCompressDevice> mMockedProbeExtractDevice;
    mutable std::vector<std::unique_ptr<MockedCompressDevice>> mMockedProbeInjectDevices;
};
}
}
}
