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

#include "cAVS/Linux/FileEntryHandler.hpp"
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

class MockedDebugFsEntryHandler final : public FileEntryHandler
{

public:
    /** Constructor
     *
     * @param[in] leftoverCallback A void(void) function that will be called if there are leftover
     *                             test inputs when destroyed.
     */
    MockedDebugFsEntryHandler(std::function<void(void)> leftoverCallback)
        : mCurrentEntry(0), mFailed(false), mLeftoverCallback(leftoverCallback)
    {
        if (!mLeftoverCallback) {
            throw std::logic_error("MockedDevice: a destruction callback must be set.");
        }
    }
    ~MockedDebugFsEntryHandler();

    /** @returns whether all test inputs have been consumed */
    bool consumed() const;

    /** Add a DebugfsEntry{OK|KO}Open entry into the test vector.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the debugfsOpen() method.
     *
     * @param[in] expectedFilename the expected filename
     * @{
     */
    void addDebugfsEntryOKOpen(const std::string &expectedFileName)
    {
        mEntries.push(std::make_unique<DebugfsEntryOpen>(expectedFileName, true));
    }
    void addDebugfsEntryKOOpen(const std::string &expectedFileName)
    {
        mEntries.push(std::make_unique<DebugfsEntryOpen>(expectedFileName, false));
    }

    /** @} */

    /** Add a successful DebugfsEntryClose entry into the test vector.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the debugfsClose() method.
     * @{
     */
    void addDebugfsEntryOKClose() { mEntries.push(std::make_unique<DebugfsEntryClose>(true)); }
    void addDebugfsEntryKOClose() { mEntries.push(std::make_unique<DebugfsEntryClose>(false)); }

    /** @} */

    /** Add a DebugfsEntry{OK|KO}Write entry into the test vector.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the debugfsWrite() method.
     *
     * @param[in] expectedBuffer the expected buffer that will be requested to write
     * @param[in] returnedSize the returned number of bytes that will be effectively written
     * @{
     */
    void addDebugfsEntryOKWrite(const util::Buffer &expectedBuffer, const ssize_t returnedSize)
    {
        mEntries.push(std::make_unique<DebugfsEntryWrite>(expectedBuffer, returnedSize, true));
    }
    void addDebugfsEntryKOWrite(const util::Buffer &expectedBuffer, const ssize_t returnedSize)
    {
        mEntries.push(std::make_unique<DebugfsEntryWrite>(expectedBuffer, returnedSize, false));
    }

    /** @} */

    /** Add a successful DebugfsEntry{OK|KO}Read entry into the test vector.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the debugfsRead() method.
     *
     * @param[in] returnedBuffer the buffer were data byte is put when reading.
     * @param[in] expectedSize the expected number of byte requested to be read that may differ of
     *                         expected read buffer
     * @param[in] returnedSize the returned number of bytes that will be effectively read
     * @{
     */
    void addDebugfsEntryOKRead(const util::Buffer &returnedBuffer, const ssize_t expectedReadSize,
                               const ssize_t returnedSize)
    {
        mEntries.push(std::make_unique<DebugfsEntryRead>(returnedBuffer, expectedReadSize,
                                                         returnedSize, true));
    }
    void addDebugfsEntryKORead(const util::Buffer &returnedBuffer, const ssize_t expectedReadSize,
                               const ssize_t returnedSize)
    {
        mEntries.push(std::make_unique<DebugfsEntryRead>(returnedBuffer, expectedReadSize,
                                                         returnedSize, false));
    }

    /** Moked open that simulates the real interface using vector entries.
     *
     * @param[in] name the filename to open
     */
    void open(const std::string &name) override;

    /** Moked close that simulates the real interface using vector entries.
     */
    void close() noexcept override;

    /** Moked write that simulates the real interface using vector entries.
     *
     * @param[in] buffer_input buffer that will be written to interface
     * @return the number of bytes that have been written from buffer to interface
     */
    ssize_t write(const util::Buffer &buffer_input) override;

    /** Moked read that simulates the real interface using vector entries.
     *
     * @param[in] buffer_output buffer pointer that will be used to put read bytes from interface
     * @param[in] nbBytes number of bytes
     * @return the number of byte that have been read from interface to buffer
     */
    ssize_t read(util::Buffer &buffer_output, const ssize_t nbBytes) override;

private:
    /** Entry DebugfsEntry class is the generic class that must be declined for each
     *  method of the interface. It contains common information.
     */
    class DebugfsEntry
    {
    public:
        DebugfsEntry(bool successsful) : mSuccesssful(successsful) {}
        bool isNotSuccessful() const { return !mSuccesssful; }

        /** destructor MUST be virtual, so DebugfsEntry is polymorphic, which allows the DynamicCast
         *  on entries.
         *  it is pure virtual as DebugfsEntry must be derived to make sense.
         */
        virtual ~DebugfsEntry() = 0;

    private:
        bool mSuccesssful; /**< vector should be successful. */
    };

    /** Entry DebugfsEntryOpen class is the class for debugfsOpen interface
     */
    class DebugfsEntryOpen : public DebugfsEntry
    {
    public:
        DebugfsEntryOpen(const std::string &expectedFilename, const bool successful)
            : DebugfsEntry(successful), mExpectedPath(expectedFilename)
        {
        }
        /** mExpectedPath : path that should be given to the open function. */
        std::string mExpectedPath;
    };
    /** Entry DebugfsEntryClose class is the class for debugfsClose interface
     *  Pretty simple as it will just check the handle when closing file.
     */
    class DebugfsEntryClose : public DebugfsEntry
    {
    public:
        DebugfsEntryClose(const bool successful) : DebugfsEntry(successful) {}
    };

    /** Entry DebugfsEntryRead class is the class for debugfsRead interface
     */
    class DebugfsEntryRead : public DebugfsEntry
    {
    public:
        DebugfsEntryRead(const util::Buffer &expectedBuffer, const ssize_t expectedReadSize,
                         const ssize_t returnedSize, const bool successful)
            : DebugfsEntry(successful), mExpectedReadSize(expectedReadSize),
              mReturnedSize(returnedSize), mExpectedOutputBuffer(expectedBuffer)
        {
        }
        ssize_t mExpectedReadSize;
        ssize_t mReturnedSize;
        /** copy of the buffer that should be returned from read function.
         * use vector that only need constructor in initialization list and no need destructors */
        util::Buffer mExpectedOutputBuffer;
    };

    /** Vector class DebugfsEntryWrite is the class vector for debugfsWrite interface
     */
    class DebugfsEntryWrite : public DebugfsEntry
    {
    public:
        DebugfsEntryWrite(const util::Buffer &expectedBuffer, const ssize_t returnedSize,
                          bool successful)
            : DebugfsEntry(successful), mReturnedSize(returnedSize),
              mExpectedInputBuffer(expectedBuffer)
        {
        }
        ssize_t mReturnedSize;

        /**  mExpectedInputBuffer : copy of the buffer passed to write function. */
        util::Buffer mExpectedInputBuffer;
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
    using DebugfsEntryPtr = std::unique_ptr<DebugfsEntry>;
    using EntryCollection = std::queue<DebugfsEntryPtr>;
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
