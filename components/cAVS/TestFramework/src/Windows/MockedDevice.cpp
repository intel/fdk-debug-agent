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

    if (successsful) {

        /* If the ioctl is successful:
         * - expectedOutputBuffer and returnedOutputBuffer have to be both null or both not null
         * - if they are not null, expectedOutputBuffer should have the same size than
         *   returnedOutputBuffer.
         */

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
    else {
        /* If the ioctl will fail, returnedOutputBuffer is not used because nothing is returned. */

        /** Guaranteed by the addFailedIoctlEntry method */
        assert(returnedOutputBuffer == nullptr);

        if (expectedOutputBuffer != nullptr) {
            mExpectedOutputBuffer = std::make_shared<Buffer>(*expectedOutputBuffer);
        }
    }
}

MockedDevice::~MockedDevice()
{
    if (!mFailed && mCurrentEntry != mEntries.size()) {
        throw Exception("IoCtl test vector has not been fully consumed.");
    }
}

void MockedDevice::addSuccessfulIoctlEntry(uint32_t ioControlCode, const Buffer *expectedInput,
    const Buffer *expectedOutput, const Buffer *returnedOutput)
{
    mEntries.push_back(IoCtlEntry(
        ioControlCode,
        expectedInput,
        expectedOutput,
        returnedOutput,
        true));
}

void MockedDevice::addFailedIoctlEntry(uint32_t ioControlCode, const Buffer *expectedInput,
    const Buffer *expectedOutput)
{
    mEntries.push_back(IoCtlEntry(
        ioControlCode,
        expectedInput,
        expectedOutput,
        nullptr,
        false));
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

    /* Checking input buffer content */
    compareBuffers("Input buffer", input, entry.getExpectedInputBuffer());

    /* Checking output buffer content */
    compareBuffers("Output buffer", output, entry.getExpectedOutputBuffer());

    /* Incrementing entry index */
    mCurrentEntry++;

    /* Simulate failure if the entry specifies it. */
    if (!entry.isSuccessful()) {
        throw Exception("Mock specifies failure.");
    }

    /* Setting the returned output buffer if it exists */
    if (entry.getReturnedOutputBuffer() != nullptr)
    {
        /* Guaranteed because entry.getReturnedOutputBuffer() != null
         * => implies that entry.getExpectedOutputBuffer() != null
         * => implies that output == entry.getExpectedOutputBuffer()
         * => implies that output != null
         */
        assert(output != nullptr);

        /* Guaranteed because candidateOutputBuffer == expectedOutputBuffer and
         * expectedOutputBuffer.size() == returnedOutputBuffer.size()
         * therefore output.size() == returnedOutputBuffer.size()
         */
        assert(output->getSize() == entry.getReturnedOutputBuffer()->getSize());

        /*Now copying the returned buffer into the candidate output buffer * /
        /* @todo: find a cross-OS memcpy replacement to avoid klockwork error */
        std::memcpy(output->getPtr(), entry.getReturnedOutputBuffer()->getPtr(),
            entry.getReturnedOutputBuffer()->getSize());
    }
}

void MockedDevice::compareBuffers(
    const std::string &bufferName,
    const Buffer *candidateBuffer,
    const Buffer *expectedBuffer)
{
    if (candidateBuffer != nullptr) {
        if (expectedBuffer != nullptr) {

            /* Checking size */
            if (candidateBuffer->getSize() != expectedBuffer->getSize()) {
                entryFailure(bufferName+" candidate with size " +
                    std::to_string(candidateBuffer->getSize()) +
                    " differs from required size: " +
                    std::to_string(expectedBuffer->getSize()));
            }


            /* Checking buffer content */
            if (*candidateBuffer != *expectedBuffer) {
                entryFailure(bufferName + " content is not the expected one.");
            }
        }
        else
        {
            /* Input buffer is not null and expected input buffer is null*/
            entryFailure(bufferName + " should be null.");
        }
    }
    else
    {
        if (expectedBuffer != nullptr) {
            /* Input buffer is null and expected input buffer is not null*/
            entryFailure(bufferName + " should not be null.");
        }
    }
}

}
}
}
