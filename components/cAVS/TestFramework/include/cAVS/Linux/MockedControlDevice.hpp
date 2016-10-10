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

#include "cAVS/Linux/ControlDevice.hpp"
#include "cAVS/Linux/ControlDeviceTypes.hpp"
#include "Util/EnumHelper.hpp"
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
class MockedControlDevice final : public ControlDevice
{
public:
    enum class Command
    {
        Read,
        Write
    };
    static const util::EnumHelper<Command> &CommandHelper()
    {
        static const util::EnumHelper<Command> helper({
            {Command::Read, "Read"}, {Command::Write, "Write"},
        });
        return helper;
    }

    /** Constructor
     *
     * @param[in] leftoverCallback A void(void) function that will be called if there are leftover
     *                             test inputs when destroyed.
     */
    MockedControlDevice(const std::string &name, std::function<void(void)> leftoverCallback)
        : ControlDevice(name), mFailed(false), mLeftoverCallback(leftoverCallback)
    {
    }
    ~MockedControlDevice() override;

    void ctlRead(const std::string &name, util::Buffer &bufferOutput) override;
    void ctlWrite(const std::string &name, const util::Buffer &bufferInput) override;

    size_t getControlCountByTag(const std::string & /*tag*/) const override
    {
        /** used to get max probe count in extraction and in injection.
         * So answer 4 (4 for extract, 4 for injection to get 8 probe point...). */
        return 4;
    }

    /** @returns whether all test inputs have been consumed */
    bool consumed() const;

    /** Add a read control entry into the test queue.
     *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
     *  - if an output buffer is required, the 'expectedOutput' AND 'returnedOutput' arguments
     *    shall be specified. They must have the same size.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the ioControl() method.
     *
     * @param[in] isSuccessful true if read is expected to be succesful, false otherwise
     * @param[in] controlName the expected control name
     * @param[in] expectedOutput the expected output buffer (because output buffer can be used
     *                           also as input buffer)
     * @param[in] returnedOutput the returned buffer
     * @throw Device::Exception
     *
     * Note: Supplied buffers are cloned, ownership is not transferred.
     */
    void addControlReadEntry(bool isSuccessful, const std::string &controlName,
                             const util::Buffer &expectedOutput,
                             const util::Buffer &returnedOutput);

    /** Add a successful read control entry into the test queue.
     *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
     *  - if an output buffer is required, the 'expectedOutput' AND 'returnedOutput' arguments
     *    shall be specified. They must have the same size.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the ioControl() method.
     *
     * @param[in] controlName the expected control name
     * @param[in] expectedOutput the expected output buffer (because output buffer can be used
     *                           also as input buffer)
     * @param[in] returnedOutput the returned buffer
     * @throw Device::Exception
     *
     * Note: Supplied buffers are cloned, ownership is not transferred.
     */
    void addSuccessfulControlReadEntry(const std::string &controlName,
                                       const util::Buffer &expectedOutput,
                                       const util::Buffer &returnedOutput);

    /** Add a failed read control entry into the test queue.
    *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
    *  - if an output buffer is required, the 'expectedOutput' argument shall be specified.
    *
    * All entries are added in an ordered way, and will be consumed in the same order
    * when using the ioControl() method.
    *
    * @param[in] controlName the expected control name
    * @param[in] expectedInput the expected input buffer
    * @param[in] expectedOutput the expected output buffer (because output buffer can be used
    *                           also as input buffer)
    * @throw Device::Exception
    *
    * Note: Supplied buffers are cloned, ownership is not transferred.
    */
    void addFailedControlReadEntry(const std::string &controlName,
                                   const util::Buffer &expectedOutput,
                                   const util::Buffer &returnedOutput);

    /** Add a successful write control entry into the test queue.
     *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
     *  - if an output buffer is required, the 'expectedOutput' AND 'returnedOutput' arguments
     *    shall be specified. They must have the same size.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the ioControl() method.
     *
     * @param[in] isSuccessful true if write is expected to be succesful, false otherwise
     * @param[in] controlName the expected control name
     * @param[in] expectedInput the expected input buffer
     * @throw Device::Exception
     *
     * Note: Supplied buffers are cloned, ownership is not transferred.
     */
    void addControlWriteEntry(bool isSuccessful, const std::string &controlName,
                              const util::Buffer &expectedInput);

    /** Add a successful write control entry into the test queue.
     *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
     *  - if an output buffer is required, the 'expectedOutput' AND 'returnedOutput' arguments
     *    shall be specified. They must have the same size.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the ioControl() method.
     *
     * @param[in] controlName the expected control name
     * @param[in] expectedInput the expected input buffer
     * @throw Device::Exception
     *
     * Note: Supplied buffers are cloned, ownership is not transferred.
     */
    void addSuccessfulControlWriteEntry(const std::string &controlName,
                                        const util::Buffer &expectedInput);

    /** Add a failed write control entry into the test queue.
    *  - if an input buffer is required, the 'expectedInput' argument shall be specified.
    *  - if an output buffer is required, the 'expectedOutput' argument shall be specified.
    *
    * All entries are added in an ordered way, and will be consumed in the same order
    * when using the ioControl() method.
    *
    * @param[in] controlName the expected control name
    * @param[in] expectedInput the expected input buffer
    * @throw Device::Exception
    *
    * Note: Supplied buffers are cloned, ownership is not transferred.
    */
    void addFailedControlWriteEntry(const std::string &controlName,
                                    const util::Buffer &expectedInput);

private:
    void ctlRW(Command command, const std::string &controlName, const util::Buffer &bufferInput,
               util::Buffer &bufferOutput);

    /** An control entry, which is a 5-tuple */
    class ControlEntry final
    {
    public:
        ControlEntry(const std::string &controlName, const util::Buffer &expectedInputBuffer,
                     bool successsful);

        ControlEntry(const std::string &controlName, const util::Buffer &expectedOutputBuffer,
                     const util::Buffer &returnedOutputBuffer, bool successsful);

        std::string getControlName() const { return mControlName; }

        Command getCommand() const { return mCommand; }

        const util::Buffer getExpectedInputBuffer() const { return mExpectedInputBuffer; }

        const util::Buffer getExpectedOutputBuffer() const { return mExpectedOutputBuffer; }

        const util::Buffer getReturnedOutputBuffer() const { return mReturnedOutputBuffer; }

        bool isSuccessful() const { return mSuccesssful; }

    private:
        /** Supplied buffers are cloned, ownership is not transferred. */
        ControlEntry(Command command, const std::string &controlName,
                     const util::Buffer &expectedInputBuffer,
                     const util::Buffer &expectedOutputBuffer,
                     const util::Buffer &returnedOutputBuffer, bool successsful);

        Command mCommand;
        std::string mControlName;
        util::Buffer mExpectedInputBuffer;
        util::Buffer mExpectedOutputBuffer;
        util::Buffer mReturnedOutputBuffer;
        bool mSuccesssful;
    };

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
        failure("Control entry #" + std::to_string(mCurrentEntry) + ": " + msg);
    }

    void checkNonFailure()
    {
        if (mFailed) {
            throw Exception("Mock failed: " + mFailureMessage);
        }
    }

    /** Compare two buffers, each buffer can be null */
    void compareBuffers(const std::string &bufferName, const util::Buffer &candidateBuffer,
                        const util::Buffer &expectedBuffer);

    using EntryCollection = std::queue<ControlEntry>;

    EntryCollection mEntries;
    int mCurrentEntry;
    bool mFailed;
    std::function<void(void)> mLeftoverCallback;
    std::string mFailureMessage;

    /* A device supports concurent control calls */
    std::mutex mMemberMutex;
};
}
}
}
