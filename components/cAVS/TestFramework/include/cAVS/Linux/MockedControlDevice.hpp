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
