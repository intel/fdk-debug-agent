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
