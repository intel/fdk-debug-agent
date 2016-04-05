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

#include "cAVS/Windows/EventHandle.hpp"
#include "Util/RingBufferReader.hpp"
#include "Util/Buffer.hpp"
#include "Util/MemoryStream.hpp"

#include <string>
#include <iostream>
#include <memory>
#include <deque>
#include <cstdint>

namespace debug_agent
{
namespace cavs
{
namespace windows
{
namespace probe
{

/**
 * This input stream extracts data from the probe extraction ring buffer
 * It uses an event handle to know when the ring buffer is filled.
 */
class ExtractionInputStream : public util::InputStream
{
public:
    using Buffer = util::Buffer;

    /**
     * @param[in,out] handle A windows event handle to know when the ring buffer has been filled
     * @param[in,out] ringBuffer The probe extraction ring buffer
     */
    ExtractionInputStream(EventHandle &handle, util::RingBufferReader &ringBuffer)
        : mCurentBlockStream(mCurrentBlock), mHandleWaiter(handle), mRingBuffer(ringBuffer)
    {
    }

    /** Close the stream, leading to unblock the thread that is reading */
    void close() { mHandleWaiter.stopWait(); }

    std::size_t read(util::StreamByte *begin, std::size_t byteCount) override
    {
        const util::StreamByte *end = begin + byteCount;
        util::StreamByte *current = begin;

        while (current < end) {

            // if current block has been fully read, fetching another one
            if (mCurentBlockStream.isEOS()) {
                if (!fetchNextBlock()) {
                    // end of stream is reached
                    return current - begin;
                }
            }

            // trying to read remaining bytes from the current block
            std::size_t remaining = end - current;
            std::size_t nread = mCurentBlockStream.read(current, remaining);
            current += nread;
        }
        return byteCount;
    }

private:
    bool fetchNextBlock()
    {
        if (not mHandleWaiter.wait()) {
            /** Event handle is closed */
            return false;
        }

        mCurrentBlock.clear();
        mRingBuffer.readAvailable(mCurrentBlock);
        mCurentBlockStream.reset();
        return true;
    }

    util::Buffer mCurrentBlock;

    /** Input stream that reads from the current block */
    util::MemoryInputStream mCurentBlockStream;
    EventHandle::Waiter mHandleWaiter;
    util::RingBufferReader &mRingBuffer;
};
}
}
}
}
