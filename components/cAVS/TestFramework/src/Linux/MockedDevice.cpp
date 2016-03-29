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

#include "cAVS/Linux/MockedDevice.hpp"
#include "Util/AssertAlways.hpp"
#include <cassert>

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

ssize_t MockedDevice::commandWrite(const std::string &name, const util::Buffer &bufferInput)
{
    checkNonFailure();
    /* Several threads can call this method, so protecting against it.
     *
     * Note: although this mutex protects against concurrent calls,
     *       no concurrent calls should happen because this leads to randomize the call order
     *       which won't match probably the test vector.
     */
    std::lock_guard<std::mutex> locker(mMemberMutex);

    /* Checking that the test vector is not already consumed */
    if (consumed()) {
        failure("Debugfs vector already consumed.");
    }
    CommandEntryPtr entryPtr(std::move(mEntries.front()));
    const CommandWriteEntry *entry = dynamic_cast<CommandWriteEntry *>(entryPtr.get());
    if (entry == nullptr) {
        failure("Wrong debugfsRead method, expecting another command.");
    }

    if (name != entry->mExpectedPath) {
        failure(std::string("Wrong expected path. Expecting :") + name + " , having: " +
                entry->mExpectedPath);
    }
    if (bufferInput.size() != entry->mExpectedInputBuffer.size()) {
        failure(std::string("Wrong expected size for write. Expecting :") +
                std::to_string(bufferInput.size()) + " , having: " +
                std::to_string(entry->mExpectedInputBuffer.size()));
    }

    if (not std::equal(entry->mExpectedInputBuffer.begin(), entry->mExpectedInputBuffer.end(),
                       bufferInput.data())) {
        failure("Buffer is different than expected.");
    }
    mEntries.pop();
    mCurrentEntry++;

    if (entry->isNotSuccessful()) {
        throw Exception("error during commandWrite: error#MockDevice");
    }
    return entry->mReturnedSize; /** May be different from buffer size depending of vector */
}

void MockedDevice::commandRead(const std::string &name, const util::Buffer &bufferInput,
                               util::Buffer &bufferOutput)
{
    checkNonFailure();
    /* Several threads can call this method, so protecting against it.
     *
     * Note: although this mutex protects against concurrent calls,
     *       no concurrent calls should happen because this leads to randomize the call order
     *       which won't match probably the test vector.
     */
    std::lock_guard<std::mutex> locker(mMemberMutex);

    /* Checking that the test vector is not already consumed */
    if (consumed()) {
        failure("Debugfs vector already consumed.");
    }
    CommandEntryPtr entryPtr(std::move(mEntries.front()));
    const CommandReadEntry *entry = dynamic_cast<CommandReadEntry *>(entryPtr.get());
    if (entry == nullptr) {
        failure("Wrong debugfsRead method, expecting another command.");
    }

    if (name != entry->mExpectedPath) {
        failure(std::string("Wrong expected path. Expecting :") + name + " , having: " +
                entry->mExpectedPath);
    }
    if (bufferInput.size() != entry->mExpectedInputBuffer.size()) {
        failure(std::string("Wrong expected size for write. Expecting :") +
                std::to_string(bufferInput.size()) + " , having: " +
                std::to_string(entry->mExpectedInputBuffer.size()));
    }

    if (not std::equal(entry->mExpectedInputBuffer.begin(), entry->mExpectedInputBuffer.end(),
                       bufferInput.data())) {
        failure("Buffer is different than expected.");
    }
    if (bufferOutput.size() != entry->mExpectedOutputBuffer.size()) {
        failure(std::string("Wrong expected size for write. Expecting :") +
                std::to_string(bufferOutput.size()) + " , having: " +
                std::to_string(entry->mExpectedOutputBuffer.size()));
    }

    if (not std::equal(entry->mExpectedOutputBuffer.begin(), entry->mExpectedOutputBuffer.end(),
                       bufferOutput.data())) {
        failure("Buffer is different than expected.");
    }

    bufferOutput = entry->mExpectedReturnedBuffer;

    mEntries.pop();
    mCurrentEntry++;

    if (entry->isNotSuccessful()) {
        throw Exception("error during commandRead: error#MockDevice");
    }
}

/** Destructor of DebugfsEntry is required so the pure virtual destuctor exist for linking
 *  (called by derived classes of DebugfsEntry destructors, if it doesn't exist you can't link)
 */
MockedDevice::CommandEntry::~CommandEntry() = default;

MockedDevice::~MockedDevice()
{
    if (!consumed()) {
        mLeftoverCallback();
    }
}

bool MockedDevice::consumed() const
{
    return mEntries.empty();
}
}
}
}
