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

    /** Add a DebugfsEntry{OK|KO}Open entry into the test vector.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the debugfsOpen() method.
     *
     * @param[in] expectedFilename the expected filename
     * @{
     */
    void addDebugfsEntryOKOpen(const std::string &expectedFilename);
    void addDebugfsEntryKOOpen(const std::string &expectedFilename);
    /** @} */

    /** Add a successful DebugfsEntryClose entry into the test vector.
     *
     * All entries are added in an ordered way, and will be consumed in the same order
     * when using the debugfsClose() method.
     * @{
     */
    void addDebugfsEntryOKClose();
    void addDebugfsEntryKOClose();
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
    void addDebugfsEntryOKWrite(const util::Buffer &expectedBuffer, const ssize_t returnedSize);
    void addDebugfsEntryKOWrite(const util::Buffer &expectedBuffer, const ssize_t returnedSize);
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
                               const ssize_t returnedSize);
    void addDebugfsEntryKORead(const util::Buffer &returnedBuffer, const ssize_t expectedReadSize,
                               const ssize_t returnedSize);

    /** Add a successful DeviceCorePowerCommand{OK|KO}Read entry into the test vector.
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
    void addDeviceCorePowerCommandOK(unsigned int coreId, bool allowedToSleep)
    {
        mEntries.push(std::make_unique<DeviceCorePowerCommand>(coreId, allowedToSleep, true));
    }

    void addDeviceCorePowerCommandKO(unsigned int coreId, bool allowedToSleep)
    {
        mEntries.push(std::make_unique<DeviceCorePowerCommand>(coreId, allowedToSleep, false));
    }

    void setCorePowerState(unsigned int coreId, bool allowedToSleep) override;

    /** Moked debugfsOpen that simulates the real interface using vector entries.
     *
     * @param[in] name the filename to open
     */
    void debugfsOpen(const std::string &name);

    /** Moked debugfsClose that simulates the real interface using vector entries.
     */
    void debugfsClose();

    /** Moked debugfsWrite that simulates the real interface using vector entries.
     *
     * @param[in] buffer_input buffer that will be written to interface
     * @return the number of bytes that have been written from buffer to interface
     */
    ssize_t debugfsWrite(const util::Buffer &buffer_input);

    /** Moked debugfsWrite that simulates the real interface using vector entries.
     *
     * @param[in] buffer_output buffer pointer that will be used to put read bytes from interface
     * @param[in] nbBytes number of bytes
     * @return the number of byte that have been read from interface to buffer
     */
    ssize_t debugfsRead(util::Buffer &buffer_output, const ssize_t nbBytes);

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

    class DeviceCorePowerCommand : public DebugfsEntry
    {
    public:
        DeviceCorePowerCommand(unsigned int coreId, bool allowedToSleep, bool successful)
            : DebugfsEntry(successful), mCoreId(coreId), mAllowedToSleep(allowedToSleep)
        {
        }
        bool allowedToSleep() const { return mAllowedToSleep; }
        unsigned int getCoreId() const { return mCoreId; }

    private:
        unsigned int mCoreId;
        bool mAllowedToSleep;
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
