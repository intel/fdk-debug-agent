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
