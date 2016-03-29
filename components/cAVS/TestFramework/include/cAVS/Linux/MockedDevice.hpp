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

#include "cAVS/Linux/Device.hpp"
#include <algorithm>
#include <memory>
#include <vector>
#include <queue>
#include <stdexcept>
#include <mutex>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class is a mocked device implementation
 *
 * Before using it, the user has to fill the test vector.
 * Each entry of this test vector is filled using functions:
 * addDebugfsEntry{OK|KO}Open
 * addDebugfsEntry{OK|KO}Close
 * addDebugfsEntry{OK|KO}Read
 * addDebugfsEntry{OK|KO}Write
 *
 * Once the test vector(s) is/are filled, the mocked device can be used as a real device.
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

    /** Add a CommandWrite{OK|KO} entry into the test vector.
    *
    * @param[in] expectedFilename the expected filename
    * @param[in] expectedBuffer the buffer expected to be written.
    * @param[in] returnedSize the returned number of bytes that will be effectively written.
    * @{
    */
    void addCommandWriteOK(const std::string &expectedFilename, const util::Buffer &expectedBuffer,
                           const ssize_t returnedSize)
    {
        mEntries.push(std::make_unique<CommandWriteEntry>(expectedFilename, expectedBuffer,
                                                          returnedSize, true));
    }

    void addCommandWriteKO(const std::string &expectedFilename, const util::Buffer &expectedBuffer,
                           const ssize_t returnedSize)
    {
        mEntries.push(std::make_unique<CommandWriteEntry>(expectedFilename, expectedBuffer,
                                                          returnedSize, false));
    }

    /** Add a CommandRead{OK|KO} entry into the test vector.
     *
     * @param[in] expectedFilename the expected filename
     * @param[in] expectedInputBuffer the buffer expected to be sent as a read command.
     * @param[in] expectedOutputBuffer the buffer expected to be returned by the read.
     * @param[in] expectedreturnedBuffer the returned number of bytes that will be effectively read
     * @{
     */
    void addCommandReadOK(const std::string &expectedFilename,
                          const util::Buffer &expectedInputBuffer,
                          const util::Buffer &expectedOutputBuffer,
                          const util::Buffer &expectedreturnedBuffer)
    {
        mEntries.push(std::make_unique<CommandReadEntry>(expectedFilename, expectedInputBuffer,
                                                         expectedOutputBuffer,
                                                         expectedreturnedBuffer, true));
    }

    void addCommandReadKO(const std::string &expectedFilename,
                          const util::Buffer &expectedInputBuffer,
                          const util::Buffer &expectedOutputBuffer,
                          const util::Buffer &expectedreturnedBuffer)
    {
        mEntries.push(std::make_unique<CommandReadEntry>(expectedFilename, expectedInputBuffer,
                                                         expectedOutputBuffer,
                                                         expectedreturnedBuffer, false));
    }

    ssize_t commandWrite(const std::string &name, const util::Buffer &bufferInput) override;

    void commandRead(const std::string &name, const util::Buffer &bufferInput,
                     util::Buffer &bufferOutput) override;

private:
    /** Entry DebugfsEntry class is the generic class that must be declined for each
     *  method of the interface. It contains common information.
     */
    class CommandEntry
    {
    public:
        CommandEntry(bool successsful) : mSuccesssful(successsful) {}
        bool isNotSuccessful() const { return !mSuccesssful; }

        /** destructor MUST be virtual, so DebugfsEntry is polymorphic, which allows the DynamicCast
         *  on entries.
         *  it is pure virtual as DebugfsEntry must be derived to make sense.
         */
        virtual ~CommandEntry() = 0;

    private:
        bool mSuccesssful; /**< vector should be successful. */
    };

    class CommandWriteEntry : public CommandEntry
    {
    public:
        CommandWriteEntry(const std::string &expectedFilename, const util::Buffer &expectedBuffer,
                          const ssize_t returnedSize, bool successful)
            : CommandEntry(successful), mReturnedSize(returnedSize),
              mExpectedPath(expectedFilename), mExpectedInputBuffer(expectedBuffer)
        {
        }
        ssize_t mReturnedSize;

        /** path that should be given to the write function. */
        std::string mExpectedPath;

        /** copy of the buffer passed to write function. */
        util::Buffer mExpectedInputBuffer;
    };

    class CommandReadEntry : public CommandEntry
    {
    public:
        CommandReadEntry(const std::string &expectedFilename,
                         const util::Buffer &expectedInputBuffer,
                         const util::Buffer &expectedOutputBuffer,
                         const util::Buffer &expectedReturnedBuffer, bool successful)
            : CommandEntry(successful), mExpectedPath(expectedFilename),
              mExpectedInputBuffer(expectedInputBuffer),
              mExpectedOutputBuffer(expectedOutputBuffer),
              mExpectedReturnedBuffer(expectedReturnedBuffer)
        {
        }
        /** path that should be given to the read function. */
        std::string mExpectedPath;

        /** copy of the buffer passed to read function. */
        util::Buffer mExpectedInputBuffer;
        util::Buffer mExpectedOutputBuffer;
        util::Buffer mExpectedReturnedBuffer;
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

    /** Call this method in case of mock failure. The current test vector entry is printed. */
    void entryFailure(const std::string &msg)
    {
        failure("DebugFS entry #" + std::to_string(mCurrentEntry) + ": " + msg);
    }

    void checkNonFailure()
    {
        if (mFailed) {
            throw Exception("Mock failed: " + mFailureMessage);
        }
    }
    using CommandEntryPtr = std::unique_ptr<CommandEntry>;
    using EntryCollection = std::queue<CommandEntryPtr>;
    EntryCollection mEntries;

    std::size_t mCurrentEntry;
    bool mFailed;
    std::string mFailureMessage;

    /* A device supports concurent file manipulation calls */
    std::mutex mMemberMutex;
    std::function<void(void)> mLeftoverCallback;
};
}
}
}
