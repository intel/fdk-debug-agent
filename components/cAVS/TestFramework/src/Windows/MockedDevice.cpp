/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
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

#include "cAVS/Windows/MockedDevice.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/* Io ctl entry constructor*/
MockedDevice::IoCtlEntry::IoCtlEntry(uint32_t ioControlCode, const Buffer *expectedInputBuffer,
        const Buffer *expectedOutputBuffer, const Buffer *returnedOutputBuffer,
        bool successsful) :
        mIoControlCode(ioControlCode), mSuccesssful(successsful)
{
    if (expectedInputBuffer != nullptr) {
        mExpectedInputBuffer = std::make_shared<Buffer>(*expectedInputBuffer);
    }

    if ((expectedOutputBuffer != nullptr) != (returnedOutputBuffer != nullptr)) {
        throw Exception("Expected buffer and returned buffer have to be both null or "
            "both not null");
    }

    if (expectedOutputBuffer != nullptr)
    {
        /* Because of previous check it's sure that returnedOutputBuffer != nullptr */
        assert(returnedOutputBuffer != nullptr);

        if (expectedOutputBuffer->getSize() != returnedOutputBuffer->getSize()) {
            throw Exception("Expected buffer and returned buffer shall have the same size");
        }

        mExpectedOutputBuffer = std::make_shared<Buffer>(*expectedOutputBuffer);
        mReturnedOutputBuffer = std::make_shared<Buffer>(*returnedOutputBuffer);
    }
}

MockedDevice::~MockedDevice()
{
    if (!mFailed && mCurrentEntry != mEntries.size()) {
        throw Exception("IoCtl test vector has not been fully consumed.");
    }
}

void MockedDevice::addIoctlEntry(uint32_t ioControlCode, const Buffer *expectedInput,
    const Buffer *expectedOutput, const Buffer *returnedOutput, bool success)
{
    mEntries.push_back(IoCtlEntry(
        ioControlCode,
        expectedInput,
        expectedOutput,
        returnedOutput,
        success));
}

void MockedDevice::ioControl(uint32_t ioControlCode, const Buffer *input, Buffer *output)
{
    checkNonFailure();

    /* Checking that the test vector is not already consumed */
    if (mCurrentEntry >= mEntries.size()) {
        failure("IoCtl vector already consumed.");
    }

    /* Getting the current entry */
    const IoCtlEntry &entry = mEntries[mCurrentEntry];

    /* Checking io control code */
    if (ioControlCode != entry.getIOControlCode()) {
        entryFailure("IoCtrl code: " + std::to_string(ioControlCode) + " expected : " +
            std::to_string(entry.getIOControlCode()));
    }

    /* Checking input buffer */
    checkInputBuffer(input, entry.getExpectedInputBuffer());

    /* Checking and setting output buffer */
    checkAndSetOutputBuffer(output, entry.getExpectedOutputBuffer(),
        entry.getReturnedOutputBuffer());


    /* Incrementing entry index */
    mCurrentEntry++;

    /* Simulate failure if the entry specifies it. */
    if (!entry.isSuccessful()) {
        throw Exception("Mock specifies failure.");
    }
}

void MockedDevice::checkInputBuffer(const Buffer *candidateInputBuffer,
    const Buffer *expectedInputBuffer)
{
    if (candidateInputBuffer != nullptr) {
        if (expectedInputBuffer != nullptr) {
            /* Checking buffer content */
            if (*candidateInputBuffer != *expectedInputBuffer) {
                entryFailure("Input buffer content is not the expected one.");
            }
        }
        else
        {
            /* Input buffer is not null and expected input buffer is null*/
            entryFailure("Input buffer should be null.");
        }
    }
    else
    {
        if (expectedInputBuffer != nullptr) {
            /* Input buffer is null and expected input buffer is not null*/
            entryFailure("Input buffer should not be null.");
        }
    }
}

void MockedDevice::checkAndSetOutputBuffer(Buffer *candidateOutputBuffer,
    const Buffer *expectedOutputBuffer, const Buffer *returnedOutputBuffer)
{
    /* Checking output buffer, which can be used as input buffer */
    if (candidateOutputBuffer != nullptr) {
        if (expectedOutputBuffer != nullptr) {

            /* This has already been checked when adding the ioctl entry */
            assert(returnedOutputBuffer != nullptr);
            assert(expectedOutputBuffer->getSize() == returnedOutputBuffer->getSize());

            /* Checking the size*/
            if (candidateOutputBuffer->getSize() != expectedOutputBuffer->getSize()) {
                entryFailure("Candidate output buffer size " +
                    std::to_string(candidateOutputBuffer->getSize()) +
                    " differs from required size: " +
                    std::to_string(expectedOutputBuffer->getSize()));
            }

            /* Checking the content*/
            if (*candidateOutputBuffer != *expectedOutputBuffer) {
                entryFailure("Output buffer content is not the expected one.");
            }

            /* Now copying the returned buffer into the candidate output buffer */
            /* @todo: find a cross-OS memcpy replacement to avoid klockwork error */
            std::memcpy(candidateOutputBuffer->getPtr(), returnedOutputBuffer->getPtr(),
                returnedOutputBuffer->getSize());
        }
        else {
            /* Output buffer is not null and expected output buffer is null*/
            entryFailure("Output buffer should be null.");
        }
    }
    else {
        if (expectedOutputBuffer != nullptr) {
            /* Output buffer is null and expected output buffer is not null*/
            entryFailure("Output buffer should not be null.");
        }
    }
}


}
}
}
