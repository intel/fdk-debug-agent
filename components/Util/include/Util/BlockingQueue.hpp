/*
 * Copyright (c) 2015, Intel Corporation
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

#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <stdexcept>
#include <string>
#include <cassert>
#include <memory>

namespace debug_agent
{
namespace util
{

/**
 * This class provides a blocking queue that stores elements using std::unique_ptr.
 *
 * Threads that try to get an element with the remove() method may be blocked if the
 * queue is empty.  Then the remove() method unblocks if:
 * - an element is added
 * - the close() method is called
 *
 * A maximum memory size is specified, adding an element can fail if the maximum size
 * is reached.
 *
 * This class supports multiple producer threads and multiple consumer threads.
 *
 * @tparam T the type handled by the std::unique_ptr
 */
template <typename T>
class BlockingQueue final
{

public:
    /*
     * @param[in] maxByteSize The maximum memory size allocated to this queue
     * @param[in] elementSizeFunction A function that provides the memory size of one queue
     *                                element.
     */
    BlockingQueue(std::size_t maxByteSize,
                  std::function<std::size_t(const T &)> elementSizeFunction)
        : mMaxByteSize(maxByteSize), mElementSizeFunction(elementSizeFunction), mCurrentSize(0),
          mOpen(false)
    {
    }
    BlockingQueue(BlockingQueue &&other)
        : mMaxByteSize(other.mMaxByteSize),
          mElementSizeFunction(std::move(other.mElementSizeFunction))
    {
        std::lock_guard<std::mutex> locker(other.mMembersMutex);

        mQueue = std::move(other.mQueue);
        mCurrentSize = other.mCurrentSize;
        mOpen = other.mOpen;

        other.clearLocked();
    }
    BlockingQueue(const BlockingQueue &) = delete;
    BlockingQueue &operator=(const BlockingQueue &) = delete;
    ~BlockingQueue() { close(); }

    /** Open the queue (items can be enqueued) */
    void open()
    {
        std::lock_guard<std::mutex> locker(mMembersMutex);

        if (!mOpen) {
            mOpen = true;
        }
    }

    /* Close the queue, i.e. :
     * - no more elements can be added.
     * - then consumers are able to retrieve the remaining elements. When the queue is empty,
     *   the remove() function return nullptr.
     */
    void close()
    {
        std::lock_guard<std::mutex> locker(mMembersMutex);

        if (mOpen) {
            mOpen = false;
            mCondVar.notify_all();
        }
    }

    /**
     * Add an element.
     * @return true if the element has been successfully added.
     */
    bool add(std::unique_ptr<T> elementPtr)
    {
        assert(elementPtr != nullptr);

        std::lock_guard<std::mutex> locker(mMembersMutex);

        if (!mOpen) {
            return false;
        }

        /* Adding the element in the queue if possible */
        if (!addLocked(std::move(elementPtr))) {
            return false;
        }

        /* Waking-up only one thread that will consume the element */
        mCondVar.notify_one();
        return true;
    }

    /**
     * @return an element or nullptr if the queue is closed
     *
     * Note: This method blocks until an element is returned or if the queue is closed. In this
     *       case nullptr is returned.
     */
    std::unique_ptr<T> remove()
    {
        /* Using a std::unique_lock instead of a std::lock_guard because this lock
         * will be changed by the mCondVar.wait() method.
         */
        std::unique_lock<std::mutex> locker(mMembersMutex);

        /* Testing if the queue is closed and that all elements have been consumed */
        if (mQueue.empty() && !mOpen) {
            return nullptr;
        }

        if (mQueue.empty()) {
            /* Queue is empty: waiting... */
            mCondVar.wait(locker);

            /* Testing again if the queue is closed and that all elements have been consumed */
            if (mQueue.empty() && !mOpen) {
                return nullptr;
            }

            /** Queue can not be empty here because:
             * - either the wakeup is due to an element adding
             * - or the wakeup is due to a close() call, but in this case the previous
             *   test has returned.
             */
            assert(!mQueue.empty());
        }

        return removeLocked();
    }

    /** Clear the queue.
     *
     * All elements are deleted from memory.
     */
    void clear()
    {
        std::lock_guard<std::mutex> locker(mMembersMutex);
        clearLocked();
    }

    std::size_t getElementCount() const
    {
        std::unique_lock<std::mutex> locker(mMembersMutex);
        return mQueue.size();
    }

    std::size_t getMemorySize() const
    {
        std::unique_lock<std::mutex> locker(mMembersMutex);
        return mCurrentSize;
    }

    bool isOpen() const
    {
        std::unique_lock<std::mutex> locker(mMembersMutex);
        return mOpen;
    }

    /** Scope class that automatically opens and closes queue */
    class AutoOpenClose
    {
    public:
        AutoOpenClose(BlockingQueue<T> &queue) : mQueue(queue) { mQueue.open(); }

        ~AutoOpenClose() { mQueue.close(); }

    private:
        BlockingQueue<T> &mQueue;
    };

private:
    using QueueType = std::queue<std::unique_ptr<T>>;

    /** Add an element if possible
     *
     * Must be called in a locked context
     *
     * @return true if the element has been successfully added.
     */
    bool addLocked(std::unique_ptr<T> elementPtr)
    {
        std::size_t elementSize = mElementSizeFunction(*elementPtr);
        if (elementSize + mCurrentSize > mMaxByteSize) {
            /* Size exceeded */
            return false;
        }

        mCurrentSize += mElementSizeFunction(*elementPtr);
        assert(mCurrentSize <= mMaxByteSize);

        mQueue.push(std::move(elementPtr));

        return true;
    }

    /** Remove an element
    *
    * - Must be called in a locked context
    * - the queue must not be empty
    */
    std::unique_ptr<T> removeLocked()
    {
        assert(!mQueue.empty());

        std::unique_ptr<T> ptr = std::move(mQueue.front());

        std::size_t elementSize = mElementSizeFunction(*ptr);

        assert(elementSize <= mCurrentSize);
        mCurrentSize -= elementSize;

        mQueue.pop();

        return ptr;
    }

    /** Must be called in a locked context */
    void clearLocked()
    {
        QueueType empty;
        std::swap(mQueue, empty);
        mCurrentSize = 0;
    }

    const std::size_t mMaxByteSize;
    const std::function<std::size_t(const T &)> mElementSizeFunction;

    mutable std::mutex mMembersMutex;
    std::condition_variable mCondVar;

    QueueType mQueue;
    std::size_t mCurrentSize;
    bool mOpen;
};
}
}
