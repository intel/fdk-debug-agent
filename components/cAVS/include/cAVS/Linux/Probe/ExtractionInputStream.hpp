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
 * This input stream extracts data from the probe extraction device
 */
class ExtractionInputStream : public util::InputStream
{
public:
    ExtractionInputStream(std::unique_ptr<CompressDevice> extractionDevice)
        : mCurentBlockStream(mCurrentBlock), mProbeDevice(std::move(extractionDevice))
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

    std::size_t read(util::StreamByte *begin, std::size_t byteCount) override
    {
        const util::StreamByte *end = begin + byteCount;
        util::StreamByte *current = begin;

        while (current < end) {

            // if current block has been fully read, fetching another one
            if (mCurentBlockStream.isEOS()) {
                if (not fetchNextBlock()) {
                    // end of stream is reached or underrun
                    return current - begin;
                }
            }

            // trying to read remaining bytes from the current block
            std::size_t remaining = end - current;
            std::size_t nread = mCurentBlockStream.read(current, remaining);
            current += nread;
        }
        return byteCount;
    }

private:
    /** Open the stream, i.e. open and start the probe compress device. */
    void open()
    {
        std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
        ASSERT_ALWAYS(mProbeDevice != nullptr);

        /* First wake up associated core, or at least prevent from sleeping. */
        compress::Config config(mFragmentSize, mNbFragments);
        try {
            mProbeDevice->open(Mode::NonBlocking, compress::Role::Capture, config);
        } catch (const CompressDevice::Exception &e) {
            throw Exception("Error opening Extraction Device: " + std::string(e.what()));
        }
        try {
            mProbeDevice->start();
        } catch (const CompressDevice::Exception &e) {
            mProbeDevice->close();
            throw Exception("Error starting Extraction Device: " + std::string(e.what()));
        }
    }

    bool fetchNextBlock()
    {
        {
            std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
            if (not mProbeDevice->isRunning()) {
                /** Probe device is closed, so the input stream as well */
                return false;
            }
            mStreamBlocked = true;
        }
        bool success = false;
        try {
            success = mProbeDevice->wait(CompressDevice::mInfiniteTimeout);
        } catch (const CompressDevice::IoException &) {
            /** Log compress device has been stopped, exiting production. */
        } catch (const CompressDevice::Exception &e) {
            std::cout << "Waiting on extraction Probe Device failed " + std::string(e.what())
                      << ". Input stream closed." << std::endl;
        }
        std::lock_guard<std::mutex> locker(mProbeDeviceMutex);
        mStreamBlocked = false;
        mCondVar.notify_one();
        if (not success) {
            return false;
        }
        mCurrentBlock.clear();
        mCurrentBlock.resize(mFragmentSize);
        /* Wait guarantees that there is something to read. */
        try {
            mProbeDevice->read(mCurrentBlock);
        } catch (const CompressDevice::Exception &e) {
            std::cout << "Error reading from probe Device: " + std::string(e.what()) << std::endl;
        }
        mCurentBlockStream.reset();
        return success;
    }

    /** Fragment size is aligned with FW buffer size. */
    static const size_t mFragmentSize = 2048;
    /** Even if we could work with 2 fragments at driver side (tensed with FW ping pong buffer),
     * keep some margin to avoid xrun events. */
    static const size_t mNbFragments = 16;

    /** Current block stores each fragment read from probe compress device. */
    util::Buffer mCurrentBlock;
    /** Memory Input stream that reads from the current block */
    util::MemoryInputStream mCurentBlockStream;
    /** Probe extraction is performed by a compress device. */
    std::unique_ptr<CompressDevice> mProbeDevice;

    std::mutex mProbeDeviceMutex;
    std::condition_variable mCondVar;
    bool mStreamBlocked = false;
};
}
}
}
