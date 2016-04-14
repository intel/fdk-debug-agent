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
