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

#include "Util/Exception.hpp"
#include <cstdint>
#include <cstddef>

namespace debug_agent
{
namespace util
{

using StreamByte = uint8_t;

/** Base input stream class */
class InputStream
{
public:
    using Exception = util::Exception<InputStream>;

    InputStream() = default;
    InputStream(const InputStream &) = delete;
    InputStream &operator=(const InputStream &) = delete;
    virtual ~InputStream() = default;

    /**
     * An input stream read function is expected to block until the required bytes have been read.
     * In some case, the client of the input stream may decide to stop the ongoing read and close
     * the stream. So, upon close, if the read is blocked, it shall unblock it.
     */
    virtual void close() {}

    /**
     * This method blocks until all the required bytes are read.
     * @return the read byte count. If it is less than byteCount, end of
     *         stream is reached.
     * @throw InputStream::Exception */
    virtual std::size_t read(StreamByte *dest, std::size_t byteCount) = 0;
};

/** Base output stream class */
class OutputStream
{
public:
    using Exception = util::Exception<OutputStream>;

    OutputStream() = default;
    OutputStream(const OutputStream &) = delete;
    OutputStream &operator=(const OutputStream &) = delete;
    virtual ~OutputStream() = default;

    /** @throw OutputStream::Exception */
    virtual void write(const StreamByte *src, std::size_t byteCount) = 0;
};
}
}
