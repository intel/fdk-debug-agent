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

#include "Util/RingBufferOutputStream.hpp"
#include "Util/RingBuffer.hpp"
#include "Util/Buffer.hpp"
#include "Util/AssertAlways.hpp"
#include <future>
#include <algorithm>

namespace debug_agent
{
namespace cavs
{

/**
 * The injector is an active object that forwards data from a input ring buffer (feed by the http
 * layer) to an output ring buffer(consumed by the driver).
 *
 * If the input ring buffer is empty, silence is written to the driver ring buffer. To achieve
 * it, the injector knows the audio format sample byte size in order to not break the sample
 * alignment.
 */
class ProbeInjector
{
public:
    using Exception = util::Exception<ProbeInjector>;

    /**
     * @param[in,out] handle A windows event handle to know when the ring buffer should be filled
     * @param[in,out] ringBufferWriter The driver probe injection ring buffer
     * @param[in,out] inputRingBuffer The input ring buffer
     * @param[in] sampleByteSize The audio format sample byte size
     */
    ProbeInjector(std::unique_ptr<util::RingBufferOutputStream> rbOutputStream,
                  util::RingBuffer &inputRingBuffer, std::size_t sampleByteSize)
        : mRbOutputStream(std::move(rbOutputStream)), mInputRingBuffer(inputRingBuffer),
          mSampleByteSize(sampleByteSize)
    {
        // First pre-filling the output ring buffer
        // Note: the input buffer may already have been provisioned
        std::size_t outputBufferBytes = mRbOutputStream->getSize();
        std::size_t outputBufferSamples = outputBufferBytes / mSampleByteSize;
        injectSamples(outputBufferSamples);

        // Then starting the injector thread
        mInjectionResult = std::async(std::launch::async, &ProbeInjector::inject, this);
    }

    ProbeInjector(ProbeInjector &&) = default;
    ProbeInjector(const ProbeInjector &) = delete;
    ProbeInjector &operator=(const ProbeInjector &) = delete;
    ~ProbeInjector()
    {
        mRbOutputStream->close();

        // Clearing the input ring buffer at probe session stop instead of probe session start
        // in order to allow to provision the input ring buffer before the session start
        mInputRingBuffer.clear();
    }

private:
    void inject()
    {
        try {
            while (true) {
                if (not mRbOutputStream->wait()) {
                    /* Closed */
                    return;
                }

                // Getting available sample count from the output ring buffer for production
                std::size_t outputAvailableBytes = mRbOutputStream->getAvailable();
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
        mRbOutputStream->write(mCopyBuffer.data(), mCopyBuffer.size());
    }

    std::unique_ptr<util::RingBufferOutputStream> mRbOutputStream;
    util::RingBuffer &mInputRingBuffer;
    std::size_t mSampleByteSize;
    std::future<void> mInjectionResult;
    util::Buffer mCopyBuffer;
};
}
}
