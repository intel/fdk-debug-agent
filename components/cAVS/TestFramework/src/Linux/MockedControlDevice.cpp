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

#include "cAVS/Linux/MockedControlDevice.hpp"
#include "Util/AssertAlways.hpp"
#include <cassert>

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

static const util::Buffer emptyBuffer{};

/* Io ctl entry constructor*/
MockedControlDevice::ControlEntry::ControlEntry(Command command, const std::string &controlName,
                                                const Buffer &expectedInputBuffer,
                                                const Buffer &expectedOutputBuffer,
                                                const Buffer &returnedOutputBuffer,
                                                bool successsful)
    : mCommand(command), mControlName(controlName), mExpectedInputBuffer(expectedInputBuffer),
      mExpectedOutputBuffer(expectedOutputBuffer), mReturnedOutputBuffer(returnedOutputBuffer),
      mSuccesssful(successsful)
{
}

MockedControlDevice::ControlEntry::ControlEntry(const std::string &controlName,
                                                const util::Buffer &expectedInputBuffer,
                                                bool successsful)
    : ControlEntry(Command::Write, controlName, expectedInputBuffer, emptyBuffer, emptyBuffer,
                   successsful)
{
}

MockedControlDevice::ControlEntry::ControlEntry(const std::string &controlName,
                                                const util::Buffer &expectedOutputBuffer,
                                                const util::Buffer &returnedOutputBuffer,
                                                bool successsful)
    : ControlEntry(Command::Read, controlName, emptyBuffer, expectedOutputBuffer,
                   returnedOutputBuffer, successsful)
{
}

MockedControlDevice::~MockedControlDevice()
{
    if (!consumed()) {
        mLeftoverCallback();
    }
}

bool MockedControlDevice::consumed() const
{
    return mEntries.empty();
}

void MockedControlDevice::addControlReadEntry(bool isSuccessful, const std::string &controlName,
                                              const Buffer &expectedOutput,
                                              const Buffer &returnedOutput)
{
    /* No need to lock members, this method is called by the main thread of the test,
     * when it fills the test vector. */
    mEntries.push(ControlEntry(controlName, expectedOutput, returnedOutput, isSuccessful));
}

void MockedControlDevice::addSuccessfulControlReadEntry(const std::string &controlName,
                                                        const Buffer &expectedOutput,
                                                        const Buffer &returnedOutput)
{
    /* No need to lock members, this method is called by the main thread of the test,
     * when it fills the test vector. */
    mEntries.push(ControlEntry(controlName, expectedOutput, returnedOutput, true));
}

void MockedControlDevice::addFailedControlReadEntry(const std::string &controlName,
                                                    const Buffer &expectedOutput,
                                                    const Buffer &returnedOutput)
{
    /* No need to lock members, this method is called by the main thread of the test,
     * when it fills the test vector. */
    mEntries.push(ControlEntry(controlName, expectedOutput, returnedOutput, false));
}

void MockedControlDevice::addControlWriteEntry(bool isSuccessful, const std::string &controlName,
                                               const Buffer &expectedInput)
{
    /* No need to lock members, this method is called by the main thread of the test,
     * when it fills the test vector. */
    mEntries.push(ControlEntry(controlName, expectedInput, isSuccessful));
}

void MockedControlDevice::addSuccessfulControlWriteEntry(const std::string &controlName,
                                                         const Buffer &expectedInput)
{
    /* No need to lock members, this method is called by the main thread of the test,
     * when it fills the test vector. */
    mEntries.push(ControlEntry(controlName, expectedInput, true));
}

void MockedControlDevice::addFailedControlWriteEntry(const std::string &controlName,
                                                     const Buffer &expectedInput)
{
    /* No need to lock members, this method is called by the main thread of the test,
     * when it fills the test vector. */
    mEntries.push(ControlEntry(controlName, expectedInput, false));
}

void MockedControlDevice::ctlRead(const std::string &controlName, util::Buffer &bufferOutput)
{
    ctlRW(Command::Read, controlName, emptyBuffer, bufferOutput);
}

void MockedControlDevice::ctlWrite(const std::string &controlName, const util::Buffer &bufferInput)
{
    util::Buffer outputBuffer{};
    ctlRW(Command::Write, controlName, bufferInput, outputBuffer);
}

void MockedControlDevice::ctlRW(Command command, const std::string &controlName,
                                const util::Buffer &bufferInput, util::Buffer &bufferOutput)
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
        failure("Control vector already consumed.");
    }

    /* Getting the current entry */
    const ControlEntry entry = mEntries.front();
    mEntries.pop();
    mCurrentEntry++;

    if (command != entry.getCommand()) {
        entryFailure("Command: '" + CommandHelper().toString(command) + "', expected : '" +
                     CommandHelper().toString(entry.getCommand()) + "'");
    }
    /* Checking io control code */
    if (controlName != entry.getControlName()) {
        entryFailure("Control: '" + controlName + "', expected : '" + entry.getControlName() + "'");
    }

    /* Checking input buffer content */
    compareBuffers("Input buffer", bufferInput, entry.getExpectedInputBuffer());

    /* Checking output buffer content */
    compareBuffers("Output buffer", bufferOutput, entry.getExpectedOutputBuffer());

    /* Simulate failure if the entry specifies it. */
    if (!entry.isSuccessful()) {
        throw Exception("Control Device says that control " + CommandHelper().toString(command) +
                        " has failed.");
    }

    /* Setting the returned output buffer (ignored for a write) */

    /* The returned output buffer size may be lesser or equal than the output one's size:
         * resize the output buffer accordingly before the copy */
    bufferOutput = entry.getReturnedOutputBuffer();
}

void MockedControlDevice::compareBuffers(const std::string &bufferName,
                                         const Buffer &candidateBuffer,
                                         const Buffer &expectedBuffer)
{
    /* Checking size */
    if (candidateBuffer.size() != expectedBuffer.size()) {
        entryFailure(bufferName + " candidate with size " + std::to_string(candidateBuffer.size()) +
                     " differs from required size: " + std::to_string(expectedBuffer.size()));
    }

    /* Checking buffer content */
    if (candidateBuffer != expectedBuffer) {
        entryFailure(bufferName + " content is not the expected one.");
    }
}
}
}
}
