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

namespace debug_agent
{
namespace util
{
/** Ring Buffer output stream class is a stream with a limited size, for which write operation
 * may blocks until the requested amount of data has been written.
 * Wait / getAvailable allow to perform non blocking operation on this stream.
 */
class RingBufferOutputStream : public OutputStream
{
public:
    /**
     * As write operation may block, it allows the client to stop the stream. By closing it,
     * it shall unblock the ongoing write operation.
     */
    virtual void close() = 0;

    /**
     * As write operation may block, it allows the client to wait until data can be written
     * to the ring buffer.
     * it shall be unblocked upon close operation.
     *
     * @return true if wait is successful, ie data can now be safely written (i.e. without
     * blocking), false if failed to wait (the stream may have closed voluntarily or encountered
     * an error).
     */
    virtual bool wait() = 0;

    /**
     * As write operation may block, it allows the client to check the amount of data that
     * can be written to the ring buffer.
     * @return available free space within the ring buffer.
     */
    virtual std::size_t getAvailable() = 0;

    /**
     * @return the size of the ring buffer output stream.
     */
    virtual std::size_t getSize() const = 0;
};
}
}
