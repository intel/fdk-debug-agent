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

#include "cAVS/Windows/Device.hpp"
#include <memory>
#include <queue>
#include <stdexcept>
#include <mutex>
#include <functional>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class is a mocked device implementation
 *
 * Before using it, the user has to fill the test queue.
 * Each entry of this test queue is a 5-tuple:
 * - the expected IO control code
 * - the expected input buffer
 * - the expected output buffer (because the output buffer can be used also as input buffer).
 * - the returned output buffer
 * - the returned IO control status
 *
 * These entries can be added using the addIoctlEntry method.
 *
 * Once the test queue is filled, the mocked device can be used as a real device.
 */
class MockedDevice final : public Device
{
public:
    /** Constructor
     *
     * @param[in] leftoverCallback A void(void) function that will be called if there are leftover
     *                             test inputs when destroyed.
     */
    MockedDevice(std::function<void(void)> leftoverCallback)
        : mCurrentEntry(0), mFailed(false), mLeftoverCallback(leftoverCallback)
    {
        if (!mLeftoverCallback) {
            throw std::logic_error("MockedDevice: a destruction callback must be set.");
        }
    }
    ~MockedDevice();

    /** @returns whether all test inputs have been consumed */
    bool consumed() const;

    /** Add a successful ioctl entry into the test queue.
     *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
     *  - if an output buffer is required, the 'expectedOutput' AND 'returnedOutput' arguments
     *    shall be specified. They must have the same size.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the ioControl() method.
     *
     * @param[in] ioControlCode the expected io control code
     * @param[in] expectedInput the expected input buffer
     * @param[in] expectedOutput the expected output buffer (because output buffer can be used
     *                           also as input buffer)
     * @param[in] returnedOutput the returned buffer
     * @throw Device::Exception
     *
     * Note: Supplied buffers are cloned, ownership is not transferred.
     */
    void addSuccessfulIoctlEntry(uint32_t ioControlCode, const util::Buffer *expectedInput,
                                 const util::Buffer *expectedOutput,
                                 const util::Buffer *returnedOutput);

    /** Add a failed ioctl entry into the test queue.
    *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
    *  - if an output buffer is required, the 'expectedOutput' argument shall be specified.
    *
    * All entries are added in an ordered way, and will be consumed in the same order
    * when using the ioControl() method.
    *
    * @param[in] ioControlCode the expected io control code
    * @param[in] expectedInput the expected input buffer
    * @param[in] expectedOutput the expected output buffer (because output buffer can be used
    *                           also as input buffer)
    * @throw Device::Exception
    *
    * Note: Supplied buffers are cloned, ownership is not transferred.
    */
    void addFailedIoctlEntry(uint32_t ioControlCode, const util::Buffer *expectedInput,
                             const util::Buffer *expectedOutput);

    virtual void ioControl(uint32_t ioControlCode, const util::Buffer *input,
                           util::Buffer *output) override;

    /** Cause the debugger to break:
     * - when the matching enty is added into the vector
     * - when the matching entry is consumed from the vector
     */
    void breakOnItem(int itemIndex) { mBreakItemIndex = itemIndex; }

private:
    /** An IO control entry, which is a 5-tuple */
    class IoCtlEntry final
    {
    public:
        /** Supplied buffers are cloned, ownership is not transferred. */
        IoCtlEntry(uint32_t ioControlCode, const util::Buffer *expectedInputBuffer,
                   const util::Buffer *expectedOutputBuffer,
                   const util::Buffer *returnedOutputBuffer, bool successsful);

        uint32_t getIOControlCode() const { return mIoControlCode; }

        const util::Buffer *getExpectedInputBuffer() const { return mExpectedInputBuffer.get(); }

        const util::Buffer *getExpectedOutputBuffer() const { return mExpectedOutputBuffer.get(); }

        const util::Buffer *getReturnedOutputBuffer() const { return mReturnedOutputBuffer.get(); }

        bool isSuccessful() const { return mSuccesssful; }

    private:
        uint32_t mIoControlCode;
        std::shared_ptr<util::Buffer> mExpectedInputBuffer;
        std::shared_ptr<util::Buffer> mExpectedOutputBuffer;
        std::shared_ptr<util::Buffer> mReturnedOutputBuffer;
        bool mSuccesssful;
    };

    void addEntry(IoCtlEntry entry);

    /** Call this method in case of mock failure */
    void failure(const std::string &msg)
    {
        if (!mFailed) {
            mFailed = true;
            mFailureMessage = msg;
        }
        throw Exception("Mock failed: " + msg);
    }

    /** Call this method in case of mock failure. The current test queue entry is printed. */
    void entryFailure(const std::string &msg)
    {
        failure("IOCtl entry #" + std::to_string(mCurrentEntry) + ": " + msg);
    }

    void checkNonFailure()
    {
        if (mFailed) {
            throw Exception("Mock failed: " + mFailureMessage);
        }
    }

    /** Compare two buffers, each buffer can be null */
    void compareBuffers(const std::string &bufferName, const util::Buffer *candidateBuffer,
                        const util::Buffer *expectedBuffer);

    using EntryCollection = std::queue<IoCtlEntry>;

    EntryCollection mEntries;
    int mCurrentEntry;
    bool mFailed;
    std::function<void(void)> mLeftoverCallback;
    std::string mFailureMessage;
    int mBreakItemIndex = -1;

    /* A device supports concurent ioctl calls */
    std::mutex mMemberMutex;
};
}
}
}
