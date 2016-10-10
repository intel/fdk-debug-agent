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

#include "cAVS/Linux/SyncWait.hpp"
#include "cAVS/Linux/CompressDevice.hpp"
#include <memory>
#include <mutex>
#include <queue>
#include <unistd.h>
#include <condition_variable>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class abstracts a compress device
 */
class MockedCompressDevice final : public CompressDevice
{
public:
    /** Constructor
     *
     * @param[in] leftoverCallback A void(void) function that will be called if there are leftover
     *                             test inputs when destroyed.
     */
    MockedCompressDevice(const compress::DeviceInfo &info,
                         std::function<void(void)> leftoverCallback)
        : CompressDevice(info), mLeftoverCallback(leftoverCallback), mMockedInfo(info)
    {
    }
    ~MockedCompressDevice() override;

    void addSuccessfulCompressDeviceEntryOpen()
    {
        mEntries.push(std::make_unique<CompressOperationEntry>(true, "open"));
    }

    void addFailedCompressDeviceEntryOpen()
    {
        mEntries.push(std::make_unique<CompressOperationEntry>(false, "open"));
    }

    void addSuccessfulCompressDeviceEntryStart()
    {
        mEntries.push(std::make_unique<CompressOperationEntry>(true, "start"));
    }

    void addFailedCompressDeviceEntryStart()
    {
        mEntries.push(std::make_unique<CompressOperationEntry>(false, "start"));
    }

    void addSuccessfulCompressDeviceEntryStop()
    {
        mEntries.push(std::make_unique<CompressOperationEntry>(true, "stop"));
    }

    void addFailedCompressDeviceEntryStop()
    {
        mEntries.push(std::make_unique<CompressOperationEntry>(false, "stop"));
    }

    void addSuccessfulCompressDeviceEntryRead(const util::Buffer &returnedBuffer,
                                              size_t returnedSize)
    {
        mEntries.push(std::make_unique<CompressReadEntry>(true, returnedBuffer, returnedSize));
    }

    void addFailedCompressDeviceEntryRead(const util::Buffer &returnedBuffer, size_t returnedSize)
    {
        mEntries.push(std::make_unique<CompressReadEntry>(false, returnedBuffer, returnedSize));
    }

    void addSuccessfulCompressDeviceEntryWrite(const util::Buffer &expectedInputBuffer,
                                               size_t returnedSize)
    {
        mEntries.push(
            std::make_unique<CompressWriteEntry>(true, expectedInputBuffer, returnedSize));
    }

    void addFailedCompressDeviceEntryWrite(const util::Buffer &expectedInputBuffer,
                                           size_t returnedSize)
    {
        mEntries.push(
            std::make_unique<CompressWriteEntry>(false, expectedInputBuffer, returnedSize));
    }

    void addSuccessfulCompressDeviceEntryAvail(size_t returnedSize)
    {
        mEntries.push(std::make_unique<CompressAvailEntry>(true, returnedSize));
    }

    void addSuccessfulCompressDeviceEntryWait(int timeout, bool reply,
                                              std::shared_ptr<SyncWait> waiter = nullptr)
    {
        mEntries.push(std::make_unique<CompressWaitEntry>(true, timeout, reply, waiter));
    }

    void addFailedCompressDeviceEntryWait(int timeout, bool reply,
                                          std::shared_ptr<SyncWait> waiter = nullptr)
    {
        mEntries.push(std::make_unique<CompressWaitEntry>(false, timeout, reply, waiter));
    }

    void addSuccessfulCompressDeviceEntryGetBufferSize(size_t bufferSize)
    {
        mEntries.push(std::make_unique<CompressGetBufferSizeEntry>(true, bufferSize));
    }

    /** below are pure virtual function of Device interface */
    void open(Mode mode, compress::Role role, compress::Config &config) override;

    void close() noexcept override;

    bool isRunning() const noexcept override;

    bool isReady() const noexcept override;

    bool wait(int timeoutMs) override;

    void start() override;

    void stop() override;

    size_t write(const util::Buffer &inputBuffer) override;

    size_t read(util::Buffer &outputBuffer) override;

    const compress::DeviceInfo getInfo() const { return mMockedInfo; }

    std::size_t getAvailable() override;

    std::size_t getBufferSize() const override;

private:
    void basicMockedOperation(const std::string &function);

    /** @returns whether all test inputs have been consumed */
    bool consumed() const;

    /** A generic compress entry, to mock open, close, start, stop and wait methods. */
    class CompressOperationEntry
    {
    public:
        CompressOperationEntry(bool successsful, const std::string &op)
            : mSuccessful(successsful), mOpName(op)
        {
        }
        virtual ~CompressOperationEntry() = default;

        bool isFailing() const { return !mSuccessful; }
        const std::string getOpName() const { return mOpName; }

    private:
        bool mSuccessful; /**< vector should be successful. */
        std::string mOpName;
    };

    class CompressReadEntry final : public CompressOperationEntry
    {
    public:
        CompressReadEntry(bool successsful, const util::Buffer &returnedOutputBuffer,
                          size_t returnedSize)
            : CompressOperationEntry(successsful, "read"),
              mReturnedOutputBuffer(returnedOutputBuffer), mReturnedSize(returnedSize)
        {
        }

        const util::Buffer getReturnedOutputBuffer() const { return mReturnedOutputBuffer; }
        size_t getReturnedSize() const { return mReturnedSize; }

    private:
        util::Buffer mReturnedOutputBuffer;
        size_t mReturnedSize;
    };

    class CompressWriteEntry final : public CompressOperationEntry
    {
    public:
        CompressWriteEntry(bool successful, const util::Buffer &expectedInputBuffer,
                           size_t returnedSize)
            : CompressOperationEntry(successful, "write"),
              mExpectedInputBuffer(expectedInputBuffer), mReturnedSize(returnedSize)
        {
        }

        const util::Buffer getExpectedInputBuffer() const { return mExpectedInputBuffer; }
        size_t getReturnedSize() const { return mReturnedSize; }

    private:
        util::Buffer mExpectedInputBuffer;
        size_t mReturnedSize;
    };

    class CompressAvailEntry final : public CompressOperationEntry
    {
    public:
        CompressAvailEntry(bool successful, size_t returnedSize)
            : CompressOperationEntry(successful, "getAvailable"), mReturnedSize(returnedSize)
        {
        }
        size_t getReturnedSize() const { return mReturnedSize; }

    private:
        size_t mReturnedSize;
    };

    class CompressGetBufferSizeEntry final : public CompressOperationEntry
    {
    public:
        CompressGetBufferSizeEntry(bool successful, size_t returnedSize)
            : CompressOperationEntry(successful, "getBufferSize"), mReturnedSize(returnedSize)
        {
        }
        size_t getReturnedSize() const { return mReturnedSize; }

    private:
        size_t mReturnedSize;
    };

    class CompressWaitEntry final : public CompressOperationEntry
    {
    public:
        CompressWaitEntry(bool successful, int timeToWaitInMs, bool expectedReply,
                          std::shared_ptr<SyncWait> waiter)
            : CompressOperationEntry(successful, "wait"), mTimeoutMs(timeToWaitInMs),
              mReply(expectedReply), mWaiter(waiter)
        {
        }
        int getTimeToWaitInMs() const { return mTimeoutMs; }
        bool getReply() const { return mReply; }
        SyncWait *getSyncWait() { return mWaiter.get(); }

    private:
        int mTimeoutMs;
        bool mReply;
        std::shared_ptr<SyncWait> mWaiter;
    };

    /** Call this method in case of mock failure */
    void failure(const std::string &msg)
    {
        if (!mFailed) {
            mFailed = true;
            mFailureMessage = msg;
        }
        throw CompressDevice::Exception("Mock failed: " + msg);
    }

    /** Call this method in case of mock failure. The current test vector entry is printed. */
    void entryFailure(const std::string &msg) { failure("CompressOperationEntry entry : " + msg); }

    void checkNonFailure() const
    {
        if (mFailed) {
            throw Exception("Mock failed: " + mFailureMessage);
        }
    }

    /** Compare two buffers, each buffer can be null */
    void compareBuffers(const std::string &bufferName, const util::Buffer &candidateBuffer,
                        const util::Buffer &expectedBuffer);

    using CompressOperationEntryPtr = std::unique_ptr<CompressOperationEntry>;
    using EntryCollection = std::queue<CompressOperationEntryPtr>;
    EntryCollection mEntries;

    bool mFailed = false;
    ;
    std::string mFailureMessage;

    bool mIsRunning = false;
    bool mIsReady = false;
    mutable std::mutex mMutex;
    std::function<void(void)> mLeftoverCallback;
    compress::DeviceInfo mMockedInfo;
    std::condition_variable mCondVar;
};
}
}
}
