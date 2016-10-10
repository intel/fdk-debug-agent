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

#include "Util/Buffer.hpp"
#include "Util/Exception.hpp"

#include <string>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace debug_agent
{
namespace util
{

/** Class to abstract a ring buffer to provide safe (ie bound checked) method.
 *
 * It can only be read as it abstracts a ring-buffer which content is controlled by another party.
 * @note Currently the ring buffer contains bytes. Consider parametrizing the element_type.
 *
 * Class invariant in normal state:
 *                   producer position >= consumer position (no underflow)
 *                   producer position - consumer position <= mSize (no overflow)
 * If any of those invariants are broken, the queue will throw on any mutable operation.
 */
class RingBufferBase
{
public:
    struct Exception : public util::Exception<RingBufferBase>
    {
        using Base = util::Exception<RingBufferBase>;
        using Base::Base;

        Exception(const std::string &msg, std::size_t consumerPos, std::size_t producerPos)
            : Base(msg + " (consumer_position=" + std::to_string(consumerPos) +
                   " producer_position=" + std::to_string(producerPos) + ")")
        {
        }
    };

    /** Type of the items in the ring buffer.
    * The ring buffer is manipulated as an array of bytes.
    */
    using Sample = volatile uint8_t;

    /** The type to implement the linear position in the ring buffer.
    *
    * Linear position is not a size_t because the ring buffer loops around its bounds. Thus may be
    * writing more than the max of size_t as size_t is only require to be able to contain the size
    * of the biggest byte array allocatable).
    *
    * Note that position overflow is not checked as it should never happen. It would require
    * writing > 10,000,000TB of data for a 64 bit LinearPosition...
    */
    using LinearPosition = uint64_t;

    /** Callable that must return the current producer/consumer linear position in the ring buffer.
     * This position MUST be monotonically increasing.
     */
    using GetLinearPosition = std::function<LinearPosition(void)>;

    RingBufferBase(size_t size) : mSize(size) {}
    RingBufferBase(RingBufferBase &&) = default;
    RingBufferBase(const RingBufferBase &) = delete;
    RingBufferBase &operator=(const RingBufferBase &) = delete;

    size_t getSize() const { return mSize; }

    /**
     * @return available space from consumer position to producer one
     * @throw Exception if underflow or overflow has happened
     */
    std::size_t getAvailableConsumption()
    {
        std::size_t consumerPosition = getConsumerLinearPosition();
        std::size_t producerPosition = getProducerLinearPosition();
        if (consumerPosition > producerPosition) {
            throw Exception("Consumer position exceeds producer one", consumerPosition,
                            producerPosition);
        }
        std::size_t available = producerPosition - consumerPosition;
        if (available > getSize()) {
            throw Exception("Producer has written over consumer position", consumerPosition,
                            producerPosition);
        }

        return available;
    }

    /**
     * @return available space ahead of producer position.
     * @throw Exception if underflow or overflow has happened
     */
    std::size_t getAvailableProduction() { return getSize() - getAvailableConsumption(); }

private:
    virtual LinearPosition getConsumerLinearPosition() const = 0;
    virtual LinearPosition getProducerLinearPosition() const = 0;

    const size_t mSize;
};
}
}
