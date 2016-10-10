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
