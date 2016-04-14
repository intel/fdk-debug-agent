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
