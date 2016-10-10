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

#include "cAVS/Linux/CompressDevice.hpp"
#include <mutex>
#include <unistd.h>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class abstracts a compress device
 */
class StubbedCompressDevice : public CompressDevice
{
public:
    StubbedCompressDevice(const compress::DeviceInfo &info) : CompressDevice(info) {}

    /** below are pure virtual function of Device interface */
    void open(Mode, compress::Role, compress::Config &) override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        mIsReady = true;
    }

    void close() noexcept override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        mIsRunning = false;
        mIsReady = false;
    }

    bool wait(int timeWaitMs) override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        if (not mIsRunning || not mIsReady) {
            return false;
        }
        if (timeWaitMs < 0) {
            mCondVar.wait(locker);
            return false;
        }
        return true;
    }

    void start() override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        mIsRunning = true;
    }

    void stop() override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        if (mIsRunning) {
            mCondVar.notify_one();
        }
    }

    bool isRunning() const noexcept override { return mIsRunning; }
    bool isReady() const noexcept override { return mIsReady; }

    size_t write(const util::Buffer &inputBuffer) override { return inputBuffer.size(); }

    size_t read(util::Buffer &outputBuffer) override { return outputBuffer.size(); }

    std::size_t getAvailable() override { return 0; }

private:
    bool mIsRunning = false;
    bool mIsReady;
    std::mutex mMutex;
    std::condition_variable mCondVar;
};
}
}
}
