/*
 * Copyright (c) 2015, Intel Corporation
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

#include "Tlv/TlvResponseHandlerInterface.hpp"
#include "Util/ByteStreamReader.hpp"
#include <inttypes.h>
#include <string>
#include <stdexcept>

namespace debug_agent
{
namespace tlv
{
/**
 * Allow to read a binary TLV list and update the corresponding value using the appropriate
 * TlvResponseHandlerInterface.
 */
class TlvUnpack final
{
public:
    struct Exception final : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /**
     * Create a TlvUnpack used to read TLV from a TLV list in a binary buffer. The TLV dictionary
     * of the TlvResponseHandlerInterface is reset: each of its TLV wrapper valid flag is cleared
     * unless exception is raised.
     *
     * @param[in] responseHandler The TlvResponseHandlerInterface which describe the handled TLV
     * @param[in] buffer The TLV list buffer
     * @throw TlvUnpack::Exception
     */
    TlvUnpack(const TlvResponseHandlerInterface &responseHandler, const util::Buffer &buffer);

    /**
     * Read the next value in the TLV buffer. This method should be called until it returns false.
     * Once it has returned false, nothing more can be read and all valid flag of read values are
     * set. Even if the methods raises an exception, it can and shall be called again while it
     * returns true.
     * @return true if there might be another value to be read, false if nothing more to be read
     * @throw TlvUnpack::Exception
     */
    bool readNext();

private:
    const TlvResponseHandlerInterface &mResponseHandler;
    util::MemoryByteStreamReader mBufferReader;
};
}
}
