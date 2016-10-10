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

#include "cAVS/Windows/EventHandle.hpp"
#include "Util/RingBufferReader.hpp"
#include "Util/Buffer.hpp"
#include "Util/MemoryStream.hpp"

#include <string>
#include <iostream>
#include <memory>
#include <deque>
#include <cstdint>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
namespace probe
{

/**
 * This input stream extracts data from the probe extraction ring buffer
 * It uses an event handle to know when the ring buffer is filled.
 */
class ExtractionInputStream : public util::InputStream
{
public:
    using Buffer = util::Buffer;

    /**
     * @param[in,out] handle A windows event handle to know when the ring buffer has been filled
     * @param[in,out] ringBuffer The probe extraction ring buffer
     */
    ExtractionInputStream(EventHandle &handle, util::RingBufferReader &&ringBuffer)
        : mCurentBlockStream(mCurrentBlock), mHandleWaiter(handle),
          mRingBuffer(std::move(ringBuffer))
    {
    }

    /** Close the stream, leading to unblock the thread that is reading */
    void close() { mHandleWaiter.stopWait(); }

    std::size_t read(util::StreamByte *begin, std::size_t byteCount) override
    {
        const util::StreamByte *end = begin + byteCount;
        util::StreamByte *current = begin;

        while (current < end) {

            // if current block has been fully read, fetching another one
            if (mCurentBlockStream.isEOS()) {
                if (!fetchNextBlock()) {
                    // end of stream is reached
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
    bool fetchNextBlock()
    {
        if (not mHandleWaiter.wait()) {
            /** Event handle is closed */
            return false;
        }

        mCurrentBlock.clear();
        mRingBuffer.readAvailable(mCurrentBlock);
        mCurentBlockStream.reset();
        return true;
    }

    util::Buffer mCurrentBlock;

    /** Input stream that reads from the current block */
    util::MemoryInputStream mCurentBlockStream;
    EventHandle::Waiter mHandleWaiter;
    util::RingBufferReader mRingBuffer;
};
}
}
}
}
