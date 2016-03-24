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
