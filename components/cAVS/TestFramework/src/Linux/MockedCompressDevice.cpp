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

#include "cAVS/Linux/MockedCompressDevice.hpp"
#include "Util/AssertAlways.hpp"
#include <cassert>

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

void MockedCompressDevice::addSuccessfulCompressDeviceEntryOpen()
{
    mEntries.push(std::make_unique<CompressOperationEntry>(true, "open"));
}

void MockedCompressDevice::addFailedCompressDeviceEntryOpen()
{
    mEntries.push(std::make_unique<CompressOperationEntry>(false, "open"));
}

void MockedCompressDevice::addSuccessfulCompressDeviceEntryStart()
{
    mEntries.push(std::make_unique<CompressOperationEntry>(true, "start"));
}

void MockedCompressDevice::addFailedCompressDeviceEntryStart()
{
    mEntries.push(std::make_unique<CompressOperationEntry>(false, "start"));
}

void MockedCompressDevice::addSuccessfulCompressDeviceEntryStop()
{
    mEntries.push(std::make_unique<CompressOperationEntry>(true, "stop"));
}

void MockedCompressDevice::addFailedCompressDeviceEntryStop()
{
    mEntries.push(std::make_unique<CompressOperationEntry>(false, "stop"));
}

void MockedCompressDevice::basicMockedOperation(const std::string &func)
{
    checkNonFailure();
    /* Several threads can call this method, so protecting against it.
     *
     * Note: although this mutex protects against concurrent calls,
     *       no concurrent calls should happen because this leads to randomize the call order
     *       which won't match probably the test vector.
     */
    std::lock_guard<std::mutex> locker(mMutex);

    /* Checking that the test vector is not already consumed */
    if (consumed()) {
        failure("MockedCompressDevice vector already consumed.");
    }
    CompressOperationEntryPtr entryPtr(std::move(mEntries.front()));
    if (entryPtr == nullptr) {
        failure("Wrong CompressDevice method, invalid command.");
    }
    if (func != entryPtr->getOpName()) {
        failure("Wrong CompressDevice method, expecting " + func + "got " + entryPtr->getOpName() +
                " command.");
    }
    mEntries.pop();

    if (entryPtr->isFailing()) {
        throw Exception("error during compress " + func + ": error#MockDevice");
    }
}

/** below are pure virtual function of Device interface */
void MockedCompressDevice::open(Mode /*mode*/, Role /*role*/, compress::Config & /*config*/)
{
    basicMockedOperation(__func__);
}

void MockedCompressDevice::close() noexcept
{
}

bool MockedCompressDevice::wait(unsigned int maxWaitMs)
{
    usleep(maxWaitMs);
    return true;
}

void MockedCompressDevice::start()
{
    {
        std::unique_lock<std::mutex> locker(mMutex);
        mIsStarted = true;
    }
    basicMockedOperation(__func__);
}

void MockedCompressDevice::stop()
{
    {
        std::unique_lock<std::mutex> locker(mMutex);
        mIsStarted = false;
    }
    basicMockedOperation(__func__);
}

size_t MockedCompressDevice::write(const util::Buffer &inputBuffer)
{
    return inputBuffer.size();
}

size_t MockedCompressDevice::read(util::Buffer &outputBuffer)
{
    return outputBuffer.size();
}

MockedCompressDevice::~MockedCompressDevice()
{
    if (!consumed()) {
        mLeftoverCallback();
    }
}

bool MockedCompressDevice::consumed() const
{
    return mEntries.empty();
}
}
}
}
