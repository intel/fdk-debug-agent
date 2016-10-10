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
#include "Util/RingBufferWriter.hpp"
#include "Util/RingBuffer.hpp"
#include "Util/Buffer.hpp"
#include "Util/Stream.hpp"
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
class InjectionOutputStream final : public util::RingBufferOutputStream
{
public:
    using Exception = util::Exception<InjectionOutputStream>;

    /**
     * @param[in,out] handle A windows event handle to know when the ring buffer should be filled
     * @param[in,out] ringBufferWriter The driver probe injection ring buffer
     */
    InjectionOutputStream(EventHandle &handle, util::RingBufferWriter &&ringBufferWriter)
        : mHandleWaiter(handle), mOutputRingBuffer(std::move(ringBufferWriter))
    {
    }

    ~InjectionOutputStream() override { close(); }

    /** Close the stream, leading to unblock the thread that is writing */
    void close() { mHandleWaiter.stopWait(); }

    std::size_t getSize() const override { return mOutputRingBuffer.getSize(); }

    std::size_t getAvailable() override { return mOutputRingBuffer.getAvailableProduction(); }

    bool wait() override { return mHandleWaiter.wait(); }

    void write(const util::StreamByte *src, std::size_t byteCount) override
    {
        // writing the temporary buffer to the output ring buffer
        mOutputRingBuffer.unsafeWrite({src, src + byteCount});
    }

private:
    EventHandle::Waiter mHandleWaiter;
    util::RingBufferWriter mOutputRingBuffer;
};
}
}
}
}
