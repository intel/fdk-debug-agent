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

#include "cAVS/Windows/EventHandle.hpp"
#include "Util/RingBufferWriter.hpp"
#include "Util/RingBuffer.hpp"
#include "Util/Buffer.hpp"
#include "Util/AssertAlways.hpp"
#include <future>
#include <algorithm>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
namespace probe
{

/**
 * The injector is an active object that forwards data from a input ring buffer (feed by the http
 * layer) to an output ring buffer(consumed by the driver).
 *
 * If the input ring buffer is empty, silence is written to the driver ring buffer. To achieve
 * it, the injector knows the audio format sample byte size in order to not break the sample
 * alignment.
 */
class Injector
{
public:
    using Exception = util::Exception<Injector>;

    /**
     * @param[in,out] handle A windows event handle to know when the ring buffer should be filled
     * @param[in,out] ringBufferWriter The driver probe injection ring buffer
     * @param[in,out] inputRingBuffer The input ring buffer
     * @param[in] sampleByteSize The audio format sample byte size
     */
    Injector(EventHandle &handle, util::RingBufferWriter &&ringBufferWriter,
             util::RingBuffer &inputRingBuffer, std::size_t sampleByteSize)
        : mHandleWaiter(handle), mOutputRingBuffer(std::move(ringBufferWriter)),
          mInputRingBuffer(inputRingBuffer), mSampleByteSize(sampleByteSize)
    {
        // First pre-filling the output ring buffer
        // Note: the input buffer may already have been provisioned
        std::size_t outputBufferBytes = mOutputRingBuffer.getSize();
        std::size_t outputBufferSamples = outputBufferBytes / mSampleByteSize;
        injectSamples(outputBufferSamples);

        // Then starting the injector thread
        mInjectionResult = std::async(std::launch::async, &Injector::inject, this);
    }

    Injector(Injector &&) = default;
    Injector(const Injector &) = delete;
    Injector &operator=(const Injector &) = delete;
    ~Injector()
    {
        close();

        // Clearing the input ring buffer at probe session stop instead of probe session start
        // in order to allow to provision the input ring buffer before the session start
        mInputRingBuffer.clear();
    }

    /** Close the stream, leading to unblock the thread that is writing */
    void close() { mHandleWaiter.stopWait(); }

private:
    void inject()
    {
        try {
            util::Buffer buffer;
            while (true) {
                if (!mHandleWaiter.wait()) {
                    /* Closed */
                    return;
                }

                // Getting available sample count from the output ring buffer for production
                std::size_t outputAvailableBytes = mOutputRingBuffer.getAvailableProduction();
                std::size_t outputAvailableSamples = outputAvailableBytes / mSampleByteSize;

                if (outputAvailableSamples > 0) {
                    injectSamples(outputAvailableSamples);
                }
            }
        } catch (std::exception &e) {
            std::string message = "Aborting probe injection due to: ";
            Exception ex(message + e.what());
            std::cerr << ex.what() << std::endl;
            throw ex;
        }
    }

    void injectSamples(std::size_t sampleCount)
    {
        // resizing the temporary buffer used for copy from input ring buffer to output one
        mCopyBuffer.resize(sampleCount * mSampleByteSize);
        auto copyBufferIt = mCopyBuffer.begin();

        // copying available samples from input ring buffer
        std::size_t inputRingBufferAvailableSamples =
            mInputRingBuffer.getUsedSize() / mSampleByteSize;
        std::size_t samplesToCopy = std::min(inputRingBufferAvailableSamples, sampleCount);
        std::size_t bytesToCopy = samplesToCopy * mSampleByteSize;

        if (bytesToCopy > 0) {
            std::size_t read = mInputRingBuffer.readNonBlocking(&(*copyBufferIt), bytesToCopy);
            // Always true because available data size has been queried
            ASSERT_ALWAYS(read == bytesToCopy);
            copyBufferIt += bytesToCopy;
        }

        // completing the copy buffer with silence if needed
        if (copyBufferIt != mCopyBuffer.end()) {
            std::fill(copyBufferIt, mCopyBuffer.end(), 0);
        }

        // writing the temporary buffer to the output ring buffer
        mOutputRingBuffer.unsafeWrite(mCopyBuffer);
    }

    EventHandle::Waiter mHandleWaiter;
    util::RingBufferWriter mOutputRingBuffer;
    util::RingBuffer &mInputRingBuffer;
    std::size_t mSampleByteSize;
    std::future<void> mInjectionResult;
    util::Buffer mCopyBuffer;
};
}
}
}
}
