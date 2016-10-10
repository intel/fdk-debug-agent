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
#include "Util/AssertAlways.hpp"
#include "Util/Iterator.hpp"
#include <condition_variable>
#include <mutex>
#include <algorithm>
#include <cstdint>

namespace debug_agent
{
namespace util
{

/** Implements a ring buffer with blocking/non blocking read/write methods
 *
 * This class guarantees that underflow/overflow will not happen
 */
class RingBuffer
{
public:
    using Byte = uint8_t;

    RingBuffer(std::size_t size) : mBuffer(size) {}

    RingBuffer(RingBuffer &&other)
    {
        std::lock_guard<std::mutex> locker(other.mMemberLock);

        mOpen = other.mOpen;
        mBuffer = std::move(other.mBuffer);
        mProducerPosition = other.mProducerPosition;
        mConsumerPosition = other.mConsumerPosition;

        other.mBuffer.clear();
        other.mProducerPosition = 0;
        other.mConsumerPosition = 0;
        other.mOpen = false;

        // Making wakeup threads that are blocked on other.readBlocking() or other.writeBlocking()
        // calls
        other.mProducerVar.notify_all();
        other.mConsumerVar.notify_all();
    }

    ~RingBuffer() { close(); }

    /** Open the queue and allows production */
    void open()
    {
        std::lock_guard<std::mutex> locker(mMemberLock);
        mOpen = true;
    }

    /** Close the queue:
     * - production is no more enabled
     * - consumption is still enabled until the buffer is empty
     *
     * Note: this method makes wakup any waiting thread blocked on a write/ReadBlocking() method
     */
    void close()
    {
        std::lock_guard<std::mutex> locker(mMemberLock);
        if (mOpen) {
            mOpen = false;
            mProducerVar.notify_all();
            mConsumerVar.notify_all();
        }
    }

    /** Write content to the ring buffer and returns immediately
     *
     * @return the count of written bytes. If it less than the "count" parameter then the
     * ring buffer is full.
     */
    std::size_t writeNonBlocking(const Byte *content, std::size_t count)
    {
        std::lock_guard<std::mutex> locker(mMemberLock);
        if (!mOpen) {
            return 0;
        }
        std::size_t toWrite = std::min(getAvailableProduction(), count);
        writeLocked(content, toWrite);
        return toWrite;
    }

    /** This method blocks until the whole content is written in the ring buffer.
     * @return false if the writing has failed due to ring buffer closing
     */
    bool writeBlocking(const Byte *content, std::size_t count)
    {
        std::unique_lock<std::mutex> locker(mMemberLock);
        auto current = content;
        auto end = content + count;
        while (mOpen && current != end) {
            std::size_t remaining = end - current;
            std::size_t toWrite = std::min(getAvailableProduction(), remaining);
            if (toWrite == 0) {
                mProducerVar.wait(locker);
            } else {
                writeLocked(current, toWrite);
            }
        }
        return mOpen;
    }

    /** Read content from the ring buffer and returns immediately
     *
     * @return the count of read bytes. If it less than the "count" parameter then the
     * ring buffer is empty.
     */
    std::size_t readNonBlocking(Byte *target, std::size_t count)
    {
        std::lock_guard<std::mutex> locker(mMemberLock);
        std::size_t toRead = std::min(getAvailableConsumption(), count);
        readLocked(target, toRead);
        return toRead;
    }

    /** This method blocks until the whole content is written in the ring buffer.
     *
     * @return false if the reading has failed because the ring buffer is closed and empty.
     */
    bool readBlocking(Byte *target, std::size_t count)
    {
        std::unique_lock<std::mutex> locker(mMemberLock);
        if (!mOpen) {
            return false;
        }

        auto current = target;
        auto end = target + count;
        while (current != end && !isClosedForConsumer()) {
            std::size_t remaining = end - current;
            std::size_t toRead = std::min(getAvailableConsumption(), remaining);
            if (toRead == 0) {
                mConsumerVar.wait(locker);
            } else {
                readLocked(current, toRead);
            }
        }
        return current == end;
    }

    /** Return stored data size in the buffer */
    std::size_t getUsedSize() const
    {
        std::lock_guard<std::mutex> locker(mMemberLock);
        return getAvailableConsumption();
    }

    /** Return free data size in the buffer */
    std::size_t getAvailableSize() const
    {
        std::lock_guard<std::mutex> locker(mMemberLock);
        return getAvailableProduction();
    }

    bool isOpen() const
    {
        std::lock_guard<std::mutex> locker(mMemberLock);
        return mOpen;
    }

    void clear()
    {
        std::lock_guard<std::mutex> locker(mMemberLock);
        mProducerPosition = mConsumerPosition = 0;
    }

private:
    std::size_t getAvailableConsumption() const
    {
        ASSERT_ALWAYS(mProducerPosition >= mConsumerPosition);
        ASSERT_ALWAYS((mProducerPosition - mConsumerPosition) <= mBuffer.size());
        return mProducerPosition - mConsumerPosition;
    }

    std::size_t getAvailableProduction() const
    {
        return mBuffer.size() - getAvailableConsumption();
    }

    /** Write bytes to the ring buffer.
     *
     * @param[in,out] source The source buffer pointer. Will be incremented according to the
     *                written byte count.
     * @param[in] count the byte amount to write
     *
     * Must be called in a locked context.
     */
    void writeLocked(const Byte *&source, std::size_t count)
    {
        std::size_t producerIndex = mProducerPosition % mBuffer.size();
        auto producer = mBuffer.begin() + producerIndex;
        if (producerIndex + count <= mBuffer.size()) {
            std::copy_n(source, count, producer);
        } else {
            std::size_t firstPartSize = mBuffer.end() - producer;
            std::size_t secondPartSize = count - firstPartSize;
            std::copy_n(source, firstPartSize, producer);
            std::copy_n(source + firstPartSize, secondPartSize, mBuffer.begin());
        }
        source += count;
        mProducerPosition += count;

        mConsumerVar.notify_one();
    }

    /** Read bytes from the ring buffer.
     *
     * @param[in,out] dest The destination buffer pointer. Will be incremented according to the
     *                read byte count.
     * @param[in] count the byte amount to read
     *
     * Must be called in a locked context.
     */
    void readLocked(Byte *&dest, std::size_t count)
    {
        std::size_t consumerIndex = mConsumerPosition % mBuffer.size();
        auto consumer = mBuffer.begin() + consumerIndex;

        if (count + consumerIndex <= mBuffer.size()) {
            std::copy_n(consumer, count, MAKE_ARRAY_ITERATOR(dest, count));
        } else {
            std::size_t firstPartSize = mBuffer.end() - consumer;
            std::size_t secondPartSize = count - firstPartSize;

            std::copy_n(consumer, firstPartSize, MAKE_ARRAY_ITERATOR(dest, count));
            std::copy_n(mBuffer.begin(), secondPartSize,
                        MAKE_ARRAY_ITERATOR(dest + firstPartSize, count - firstPartSize));
        }

        dest += count;
        mConsumerPosition += count;

        mProducerVar.notify_one();
    }

    /** The buffer is closed for the consumer when it is closed AND empty.
     * This allows the consumer to retrieve data after buffer closing.
     */
    bool isClosedForConsumer() { return !mOpen && getAvailableConsumption() == 0; }

    mutable std::mutex mMemberLock;

    bool mOpen = false;
    Buffer mBuffer;
    std::size_t mProducerPosition = 0;
    std::size_t mConsumerPosition = 0;
    std::condition_variable mConsumerVar;
    std::condition_variable mProducerVar;
};
}
}
