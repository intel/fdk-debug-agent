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

void MockedDevice::addDebugfsEntryOKOpen(const std::string &expectFilename)
{
    mEntries.push(std::make_unique<DebugfsEntryOpen>(expectFilename, true));
}

void MockedDevice::addDebugfsEntryKOOpen(const std::string &expectFilename)
{
    mEntries.push(std::make_unique<DebugfsEntryOpen>(expectFilename, false));
}

void MockedDevice::addDebugfsEntryOKWrite(const Buffer &expectBuffer, const ssize_t returnSize)
{
    mEntries.push(std::make_unique<DebugfsEntryWrite>(expectBuffer, returnSize, true));
}

void MockedDevice::addDebugfsEntryKOWrite(const Buffer &expectBuffer, const ssize_t returnSize)
{
    mEntries.push(std::make_unique<DebugfsEntryWrite>(expectBuffer, returnSize, false));
}

void MockedDevice::addDebugfsEntryOKRead(const Buffer &returnBuffer, const ssize_t expectedReadSize,
                                         const ssize_t returnSize)
{
    mEntries.push(
        std::make_unique<DebugfsEntryRead>(returnBuffer, expectedReadSize, returnSize, true));
}

void MockedDevice::addDebugfsEntryKORead(const Buffer &returnBuffer, const ssize_t expectedReadSize,
                                         const ssize_t returnSize)
{
    mEntries.push(
        std::make_unique<DebugfsEntryRead>(returnBuffer, expectedReadSize, returnSize, false));
}

void MockedDevice::addDebugfsEntryOKClose()
{

    mEntries.push(std::make_unique<DebugfsEntryClose>(true));
}

void MockedDevice::addDebugfsEntryKOClose()
{

    mEntries.push(std::make_unique<DebugfsEntryClose>(false));
}

ssize_t MockedDevice::debugfsRead(util::Buffer &buffer, const ssize_t nbBytes)
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
    DebugfsEntryPtr entryPtr(std::move(mEntries.front()));
    const DebugfsEntryRead *entry = dynamic_cast<DebugfsEntryRead *>(entryPtr.get());
    if (entry == nullptr) {
        failure("Wrong debugfsRead method, expecting another command.");
    }

    if (nbBytes != entry->mExpectedReadSize) {
        failure(std::string("Wrong expected size for read. Expecting :") + std::to_string(nbBytes) +
                " , having: " + std::to_string(entry->mExpectedReadSize));
    }

    buffer = entry->mExpectedOutputBuffer;
    mEntries.pop();
    mCurrentEntry++;

    if (entry->isNotSuccessful()) {
        throw Exception("error during read: error#MockDevice");
    }
    return entry->mReturnedSize; /** May be different from nbBytes depending of vector */
}

ssize_t MockedDevice::debugfsWrite(const Buffer &buffer)
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
    DebugfsEntryPtr entryPtr(std::move(mEntries.front()));
    const DebugfsEntryWrite *entry = dynamic_cast<DebugfsEntryWrite *>(entryPtr.get());
    if (entry == nullptr) {
        failure("Wrong debugfsWrite method, expecting another command.");
    }

    if (buffer.size() != entry->mExpectedInputBuffer.size()) {
        failure(std::string("Wrong expected size for write. Expecting :") +
                std::to_string(buffer.size()) + " , having: " +
                std::to_string(entry->mExpectedInputBuffer.size()));
    }

    if (not std::equal(entry->mExpectedInputBuffer.begin(), entry->mExpectedInputBuffer.end(),
                       buffer.data())) {
        failure("Buffer is different than expected.");
    }
    mEntries.pop();
    mCurrentEntry++;

    if (entry->isNotSuccessful()) {
        throw Exception("error during write: error#MockDevice");
    }
    return entry->mReturnedSize; /** May be different from buffer size depending of vector */
}

void MockedDevice::debugfsClose()
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
    DebugfsEntryPtr entryPtr(std::move(mEntries.front()));
    const DebugfsEntryClose *entry = dynamic_cast<DebugfsEntryClose *>(entryPtr.get());
    if (entry == nullptr) {
        failure("Wrong debugfsClose method, expecting another command.");
    }

    mEntries.pop();
    mCurrentEntry++;

    if (entry->isNotSuccessful()) {
        throw Exception("error during close: error#MockDevice");
    }
}

void MockedDevice::debugfsOpen(const std::string &name)
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
    DebugfsEntryPtr entryPtr(std::move(mEntries.front()));
    const DebugfsEntryOpen *entry = dynamic_cast<DebugfsEntryOpen *>(entryPtr.get());
    if (entry == nullptr) {
        failure("Wrong debugfsOpen method, expecting another command.");
    }

    if (entry->mExpectedPath != name) {
        failure("Wrong filename for debugfsOpen method, expecting another filename.");
    }

    mEntries.pop();
    mCurrentEntry++;

    if (entry->isNotSuccessful()) {
        throw Exception("error during open: error#MockDevice");
    }
}

/** Destructor of DebugfsEntry is required so the pure virtual destuctor exist for linking
 *  (called by derived classes of DebugfsEntry destructors, if it doesn't exist you can't link)
 */
MockedDevice::DebugfsEntry::~DebugfsEntry() = default;

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
