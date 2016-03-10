/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
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
        QueueType empty;
        std::swap(mQueue, empty);
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

    BlockingQueue(const BlockingQueue &) = delete;
    BlockingQueue &operator=(const BlockingQueue &) = delete;

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
