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
