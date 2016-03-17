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

#include "Util/Stream.hpp"
#include "Util/Buffer.hpp"
#include "Util/AssertAlways.hpp"
#include "Util/Iterator.hpp"
#include <algorithm>

namespace debug_agent
{
namespace util
{

/** Input stream that reads from a memory buffer */
class MemoryInputStream : public InputStream
{
public:
    /** Ownership of the buffer is not taken. */
    MemoryInputStream(const Buffer &buffer) : mBuffer(buffer) {}

    std::size_t read(StreamByte *dest, std::size_t byteCount) override
    {
        ASSERT_ALWAYS(mIndex <= mBuffer.size());

        std::size_t toRead = std::min(byteCount, mBuffer.size() - mIndex);

        if (toRead > 0) {
            std::copy_n(mBuffer.begin() + mIndex, toRead, MAKE_ARRAY_ITERATOR(dest, toRead));

            mIndex += toRead;
        }
        return toRead;
    }

    /** @return true if stream is fully consumed, i.e. end of stream is reached */
    bool isEOS() const { return mIndex == mBuffer.size(); }

    /** @return the current stream pointer index */
    std::size_t getPointerOffset() { return mIndex; }

    void reset() { mIndex = 0; }

private:
    const Buffer &mBuffer;
    std::size_t mIndex = 0;
};

/** Output stream that writes to a memory buffer */
class MemoryOutputStream : public OutputStream
{
public:
    /** Buffer is cleared */
    MemoryOutputStream(Buffer &buffer) : mBuffer(buffer) { buffer.clear(); }

    void write(const StreamByte *src, std::size_t byteCount) override
    {
        std::size_t currentIndex = mBuffer.size();
        mBuffer.resize(mBuffer.size() + byteCount);

        std::copy(src, src + byteCount, mBuffer.begin() + currentIndex);
    }

private:
    Buffer &mBuffer;
};
}
}
