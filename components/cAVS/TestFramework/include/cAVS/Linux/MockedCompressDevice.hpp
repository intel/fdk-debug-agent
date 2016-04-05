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

#include "cAVS/Linux/CompressDevice.hpp"
#include <memory>
#include <mutex>
#include <queue>
#include <unistd.h>

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
        : CompressDevice(info), mFailed(false), mIsStarted(false),
          mLeftoverCallback(leftoverCallback), mMockedInfo(info)
    {
    }
    ~MockedCompressDevice() override;

    void addSuccessfulCompressDeviceEntryOpen();

    void addFailedCompressDeviceEntryOpen();

    void addSuccessfulCompressDeviceEntryStart();

    void addFailedCompressDeviceEntryStart();

    void addSuccessfulCompressDeviceEntryStop();

    void addFailedCompressDeviceEntryStop();

    /** below are pure virtual function of Device interface */
    void open(Mode mode, Role role, compress::Config &config) override;

    void close() noexcept override;

    bool wait(unsigned int maxWaitMs) override;

    void start() override;

    void stop() override;

    size_t write(const util::Buffer &inputBuffer) override;

    size_t read(util::Buffer &outputBuffer) override;

    const compress::DeviceInfo getInfo() const { return mMockedInfo; }

private:
    void basicMockedOperation(const std::string &function);

    /** @returns whether all test inputs have been consumed */
    bool consumed() const;

    /** A generic compress entry, to mock open, close, start, stop and wait methods. */
    class CompressOperationEntry
    {
    public:
        CompressOperationEntry(bool successsful, const std::string &op)
            : mSuccesssful(successsful), mOpName(op)
        {
        }
        virtual ~CompressOperationEntry() = default;

        bool isFailing() const { return !mSuccesssful; }
        const std::string getOpName() const { return mOpName; }

    private:
        bool mSuccesssful; /**< vector should be successful. */
        std::string mOpName;
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

    void checkNonFailure()
    {
        if (mFailed) {
            throw Exception("Mock failed: " + mFailureMessage);
        }
    }

    using CompressOperationEntryPtr = std::unique_ptr<CompressOperationEntry>;
    using EntryCollection = std::queue<CompressOperationEntryPtr>;
    EntryCollection mEntries;

    bool mFailed;
    std::string mFailureMessage;

    bool mIsStarted;
    std::mutex mMutex;
    std::function<void(void)> mLeftoverCallback;
    compress::DeviceInfo mMockedInfo;
};
}
}
}
