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

#include "Util/RingBufferBase.hpp"

namespace debug_agent
{
namespace util
{

class RingBufferReader : public RingBufferBase
{
public:
    /** Constructor
     *
     * @param bufferStart Pointer to the initial position of the ringbuffer
     * @param size The ringbuffer's size
     * @param getProducerLinearPosition The monotonic position producer
     */
    RingBufferReader(Sample *bufferStart, size_t size, GetLinearPosition getProducerLinearPosition)
        : Base(size), mStart(bufferStart),
          mGetProducerLinearPosition(std::move(getProducerLinearPosition))
    {
    }
    RingBufferReader(RingBufferReader &&) = default;

    /**
     * Read available data from ring buffer
     * The data is appended to the supplied buffer
     * @throw RingBufferBase::Exception
    */
    void readAvailable(Buffer &buffer)
    {
        std::size_t available = getAvailableConsumption();
        if (available > 0) {

            // Get the required data
            unsafeRead(available, buffer);

            // Read succeeded, commit changes
            mConsumerPosition += available;
        }
    }

private:
    using Base = RingBufferBase;
    const Sample *begin() const { return mStart; }
    const Sample *end() const { return mStart + getSize(); }
    const Sample *getConsumerPosition() const { return begin() + getConsumerOffset(); }
    size_t getConsumerOffset() const { return mConsumerPosition % getSize(); }

    void unsafeRead(size_t size, Buffer &buffer) const
    {
        buffer.reserve(buffer.size() + size); // Avoid multiple buffer growth
        if (size <= getSize() - getConsumerOffset()) {
            // Reading `size` will not buffer overflow, read the requested size
            buffer.insert(buffer.end(), getConsumerPosition(), getConsumerPosition() + size);
        } else {
            // Reading `size` will buffer overflow,
            // instead read to the end of the ring buffer
            buffer.insert(buffer.end(), getConsumerPosition(), end());
            // then read the rest at the beginning.
            buffer.insert(buffer.end(), begin(), getConsumerPosition() - (getSize() - size));
        }
    }

    LinearPosition getConsumerLinearPosition() const override { return mConsumerPosition; }

    LinearPosition getProducerLinearPosition() const override
    {
        return mGetProducerLinearPosition();
    }

    const Sample *mStart;
    const GetLinearPosition mGetProducerLinearPosition;
    LinearPosition mConsumerPosition = 0;
};
}
}
