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
#include <mutex>
#include <unistd.h>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class abstracts a compress device
 */
class StubbedCompressDevice : public CompressDevice
{
public:
    StubbedCompressDevice(const compress::DeviceInfo &info) : CompressDevice(info) {}

    /** below are pure virtual function of Device interface */
    void open(Mode, compress::Role, compress::Config &) override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        mIsReady = true;
    }

    void close() noexcept override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        mIsRunning = false;
        mIsReady = false;
    }

    bool wait(int timeWaitMs) override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        if (not mIsRunning || not mIsReady) {
            return false;
        }
        if (timeWaitMs < 0) {
            mCondVar.wait(locker);
            return false;
        }
        return true;
    }

    void start() override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        mIsRunning = true;
    }

    void stop() override
    {
        std::unique_lock<std::mutex> locker(mMutex);
        if (mIsRunning) {
            mCondVar.notify_one();
        }
    }

    bool isRunning() const noexcept override { return mIsRunning; }
    bool isReady() const noexcept override { return mIsReady; }

    size_t write(const util::Buffer &inputBuffer) override { return inputBuffer.size(); }

    size_t read(util::Buffer &outputBuffer) override { return outputBuffer.size(); }

    std::size_t getAvailable() override { return 0; }

private:
    bool mIsRunning = false;
    bool mIsReady;
    std::mutex mMutex;
    std::condition_variable mCondVar;
};
}
}
}
