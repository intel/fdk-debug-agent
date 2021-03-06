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

#include "cAVS/Windows/WindowsTypes.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include <cassert>

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/* Io ctl entry constructor*/
MockedDevice::IoCtlEntry::IoCtlEntry(uint32_t ioControlCode, const Buffer *expectedInputBuffer,
                                     const Buffer *expectedOutputBuffer,
                                     const Buffer *returnedOutputBuffer, bool successsful)
    : mIoControlCode(ioControlCode), mSuccesssful(successsful)
{
    if (expectedInputBuffer != nullptr) {
        mExpectedInputBuffer = std::make_shared<Buffer>(*expectedInputBuffer);
    }

    if (successsful) {

        /* If the ioctl is successful:
         * - expectedOutputBuffer and returnedOutputBuffer have to be both null or both not null
         * - if they are not null, expectedOutputBuffer should have the same size than
         *   returnedOutputBuffer.
         */

        if ((expectedOutputBuffer != nullptr) != (returnedOutputBuffer != nullptr)) {
            throw Exception("Expected buffer and returned buffer have to be both null or "
                            "both not null");
        }

        if (expectedOutputBuffer != nullptr) {
            /* Because of previous check it's sure that returnedOutputBuffer != nullptr */
            assert(returnedOutputBuffer != nullptr);

            if (expectedOutputBuffer->size() < returnedOutputBuffer->size()) {
                throw Exception(
                    "Expected buffer size shall be greater or equal than the returned buffer size");
            }

            mExpectedOutputBuffer = std::make_shared<Buffer>(*expectedOutputBuffer);
            mReturnedOutputBuffer = std::make_shared<Buffer>(*returnedOutputBuffer);
        }
    } else {
        /* If the ioctl will fail, returnedOutputBuffer is not used because nothing is returned. */

        /** Guaranteed by the addFailedIoctlEntry method */
        assert(returnedOutputBuffer == nullptr);

        if (expectedOutputBuffer != nullptr) {
            mExpectedOutputBuffer = std::make_shared<Buffer>(*expectedOutputBuffer);
        }
    }
}

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

void MockedDevice::addEntry(IoCtlEntry entry)
{
    mEntries.push(entry);

    // by convention first entry index is 1, so checking index after insertion
    if (mEntries.size() == mBreakItemIndex) {
        DebugBreak();
    }
}

void MockedDevice::addSuccessfulIoctlEntry(uint32_t ioControlCode, const Buffer *expectedInput,
                                           const Buffer *expectedOutput,
                                           const Buffer *returnedOutput)
{
    /* No need to lock members, this method is called by the main thread of the test,
     * when it fills the test vector. */

    addEntry(IoCtlEntry(ioControlCode, expectedInput, expectedOutput, returnedOutput, true));
}

void MockedDevice::addFailedIoctlEntry(uint32_t ioControlCode, const Buffer *expectedInput,
                                       const Buffer *expectedOutput)
{
    /* No need to lock members, this method is called by the main thread of the test,
     * when it fills the test vector. */

    addEntry(IoCtlEntry(ioControlCode, expectedInput, expectedOutput, nullptr, false));
}

void MockedDevice::ioControl(uint32_t ioControlCode, const Buffer *input, Buffer *output)
{
    /* Several threads can call this method, so protecting against it.
     *
     * Note: although this mutex protects against concurrent calls,
     *       no concurrent calls should happen because this leads to randomize the call order
     *       which won't match probably the test vector.
     */
    std::lock_guard<std::mutex> locker(mMemberMutex);

    checkNonFailure();

    /* Checking that the test vector is not already consumed */
    if (consumed()) {
        failure("IoCtl vector already consumed.");
    }

    /* Getting the current entry */
    const IoCtlEntry entry = mEntries.front();
    mEntries.pop();
    mCurrentEntry++;

    if (mCurrentEntry == mBreakItemIndex) {
        DebugBreak();
    }

    /* Checking io control code */
    if (ioControlCode != entry.getIOControlCode()) {
        entryFailure("IoCtrl code: " + std::to_string(ioControlCode) + " expected : " +
                     std::to_string(entry.getIOControlCode()));
    }

    /* Checking input buffer content */
    compareBuffers("Input buffer", input, entry.getExpectedInputBuffer());

    /* Checking output buffer content */
    compareBuffers("Output buffer", output, entry.getExpectedOutputBuffer());

    /* Simulate failure if the entry specifies it. */
    if (!entry.isSuccessful()) {
        throw Exception("OS says that io control has failed.");
    }

    /* Setting the returned output buffer if it exists */
    if (entry.getReturnedOutputBuffer() != nullptr) {
        /* Guaranteed because entry.getReturnedOutputBuffer() != null
         * => implies that entry.getExpectedOutputBuffer() != null
         * => implies that output == entry.getExpectedOutputBuffer()
         * => implies that output != null
         */
        assert(output != nullptr);

        /* The returned output buffer size may be lesser or equal than the output one's size:
         * resize the output buffer accordingly before the copy */
        *output = *entry.getReturnedOutputBuffer();
    }
}

void MockedDevice::compareBuffers(const std::string &bufferName, const Buffer *candidateBuffer,
                                  const Buffer *expectedBuffer)
{
    if (candidateBuffer != nullptr) {
        if (expectedBuffer != nullptr) {

            /* Checking size */
            if (candidateBuffer->size() != expectedBuffer->size()) {
                entryFailure(
                    bufferName + " candidate with size " + std::to_string(candidateBuffer->size()) +
                    " differs from required size: " + std::to_string(expectedBuffer->size()));
            }

            /* Checking buffer content */
            if (*candidateBuffer != *expectedBuffer) {
                entryFailure(bufferName + " content is not the expected one.");
            }
        } else {
            /* Input buffer is not null and expected input buffer is null*/
            entryFailure(bufferName + " should be null.");
        }
    } else {
        if (expectedBuffer != nullptr) {
            /* Input buffer is null and expected input buffer is not null*/
            entryFailure(bufferName + " should not be null.");
        }
    }
}
}
}
}
