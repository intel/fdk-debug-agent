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
