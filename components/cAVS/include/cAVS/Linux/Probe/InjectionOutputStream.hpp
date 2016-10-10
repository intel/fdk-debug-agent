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

#include "Util/MemoryStream.hpp"
#include "Util/RingBufferOutputStream.hpp"
#include "Util/AssertAlways.hpp"
#include "Util/BlockingQueue.hpp"
#include "Util/PointerHelper.hpp"

#include <string>
#include <iostream>
#include <memory>
#include <mutex>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/**
 * This input stream injects data to the probe injection device
 */
class InjectionOutputStream : public util::RingBufferOutputStream
{
public:
    InjectionOutputStream(std::unique_ptr<CompressDevice> injectionDevice)
        : mCurentBlockStream(mCurrentBlock), mProbeDevice(std::move(injectionDevice))
    {
        open();
    }

    /** Close the stream, i.e. stop and close the probe compress device. */
    void close() override
    {
        /* Using a std::unique_lock instead of a std::lock_guard because this lock
         * will be changed by the mCondVar.wait() method.
         */
        std::unique_lock<std::mutex> locker(mProbeDeviceMutex);
        try {
            mProbeDevice->stop();
        } catch (const CompressDevice::Exception &e) {
            /* Called from destructor, no throw... */
            std::cout << "Error stopping Probe Device: " << std::string(e.what()) << std::endl;
        }
        // Ensure we can safely close the device
        if (mStreamBlocked) {
            mCondVar.wait(locker);
        }
        ASSERT_ALWAYS(not mStreamBlocked);
        mProbeDevice->close();
    }

    void write(const util::StreamByte *src, std::size_t byteCount) override
    {
        std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
        if (not mProbeDevice->isReady()) {
            /** Probe device is closed, so the output stream as well */
            std::cout << "Error writing to injection probe device, stream closed" << std::endl;
            return;
        }
        try {
            // writing the temporary buffer to the output ring buffer
            mProbeDevice->write({src, src + byteCount});
            if (not mProbeDevice->isRunning()) {
                // As per design, shall start the device upon first write.
                mProbeDevice->start();
            }
        } catch (const CompressDevice::Exception &e) {
            std::cout << "Error writing to probe device " + std::string(e.what()) << std::endl;
        }
    }

    bool wait() override
    {
        {
            std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
            if (not mProbeDevice->isRunning()) {
                /** Probe device is closed, so the output stream as well */
                return false;
            }
            mStreamBlocked = true;
        }
        bool success = false;
        try {
            /* wait without timer, do not lock, stop will unblock us */
            success = mProbeDevice->wait(CompressDevice::mInfiniteTimeout);
        } catch (const CompressDevice::IoException &) {
            /** Log compress device has been stopped, exiting production. */
        } catch (const CompressDevice::Exception &e) {
            std::cout << "Waiting on Injection Device failed " + std::string(e.what()) << std::endl;
        }
        std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
        mStreamBlocked = false;
        mCondVar.notify_one();
        return success;
    }

    std::size_t getSize() const override
    {
        std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
        ASSERT_ALWAYS(mProbeDevice != nullptr);
        return mProbeDevice->getBufferSize();
    }

    std::size_t getAvailable() override
    {
        std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
        ASSERT_ALWAYS(mProbeDevice != nullptr);
        return mProbeDevice->getAvailable();
    }

private:
    /** Open the stream, i.e. open the probe compress device. */
    void open()
    {
        std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
        ASSERT_ALWAYS(mProbeDevice != nullptr);

        /* First wake up associated core, or at least prevent from sleeping. */
        compress::Config config(mFragmentSize, mNbFragments);
        try {
            mProbeDevice->open(Mode::NonBlocking, compress::Role::Playback, config);
        } catch (const CompressDevice::Exception &e) {
            throw Exception("Error opening Injection Device: " + std::string(e.what()));
        }
    }

    /** Fragment size is aligned with page size. */
    static const size_t mFragmentSize = 4096;
    /** Even if we could work with 2 fragments at driver side (tensed with FW ping pong buffer),
     * keep some margin to avoid xrun events. */
    static const size_t mNbFragments = 32;

    /** Current block stores each fragment read from probe compress device. */
    util::Buffer mCurrentBlock;
    /** Memory Input stream that reads from the current block */
    util::MemoryInputStream mCurentBlockStream;
    /** Probe injection is performed by a compress device. */
    std::unique_ptr<CompressDevice> mProbeDevice;

    mutable std::mutex mProbeDeviceMutex;
    std::condition_variable mCondVar;
    bool mStreamBlocked = false;
};
}
}
}
