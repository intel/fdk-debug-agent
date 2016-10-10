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
#include "Util/Iterator.hpp"
#include <algorithm>

namespace debug_agent
{
namespace util
{

class RingBufferWriter : public RingBufferBase
{
public:
    /** Constructor
     *
     * @param bufferStart Pointer to the initial position of the ringbuffer
     * @param size The ringbuffer's size
     * @param getProducerLinearPosition The monotonic position consumer
     */
    RingBufferWriter(Sample *bufferStart, size_t size, GetLinearPosition getConsumerLinearPosition)
        : Base(size), mStart(bufferStart),
          mGetConsumerLinearPosition(std::move(getConsumerLinearPosition))
    {
    }
    RingBufferWriter(RingBufferWriter &&) = default;

    /**
     * Write data to the ring buffer.
     * This writing is unsafe because this method does not check overflow. Use
     * getAvailableProduction() to get a valid size.
     */
    void unsafeWrite(const Buffer &source)
    {
        if (source.size() > getSize()) {
            throw Exception("Cannot write more data than the ring buffer size.");
        }

        if (source.size() < getSize() - getProducerOffset()) {
            std::copy(source.begin(), source.end(),
                      MAKE_ARRAY_ITERATOR(getProducerPosition(), source.size()));
        } else {
            std::size_t firstPartSize = getSize() - getProducerOffset();
            std::size_t secondPartSize = source.size() - firstPartSize;

            std::copy_n(source.begin(), firstPartSize,
                        MAKE_ARRAY_ITERATOR(getProducerPosition(), firstPartSize));
            std::copy(source.begin() + firstPartSize, source.end(),
                      MAKE_ARRAY_ITERATOR(begin(), secondPartSize));
        }

        mProducerPosition += source.size();
    }

private:
    using Base = RingBufferBase;
    Sample *begin() const { return mStart; }
    Sample *end() const { return mStart + getSize(); }
    Sample *getProducerPosition() const { return begin() + getProducerOffset(); }
    size_t getProducerOffset() const { return mProducerPosition % getSize(); }

    LinearPosition getConsumerLinearPosition() const override
    {
        return mGetConsumerLinearPosition();
    }

    LinearPosition getProducerLinearPosition() const override { return mProducerPosition; }

    Sample *mStart;
    const GetLinearPosition mGetConsumerLinearPosition;
    LinearPosition mProducerPosition = 0;
};
}
}
