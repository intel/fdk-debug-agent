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
#include "Tlv/TlvUnpack.hpp"
#include <algorithm>

namespace debug_agent
{
namespace tlv
{

TlvUnpack::TlvUnpack(const TlvResponseHandlerInterface &responseHandler, const util::Buffer &buffer)
    : mResponseHandler(responseHandler), mBufferReader(buffer)
{
    mResponseHandler.getTlvDictionary().invalidateAll();
}

bool TlvUnpack::readNext()
{
    if (mBufferReader.isEOS()) {
        return false;
    }

    util::Buffer valueBuffer;
    uint32_t tagFromBuffer;

    try {
        uint32_t lengthFromBuffer;
        mBufferReader.read(tagFromBuffer);
        mBufferReader.read(lengthFromBuffer);

        /* It is dangerous to resize the buffer to "lengthFromBuffer", because this size is
         * provided by an external component, and if this size is huge this will lead to memory
         * allocation failure.
         *
         * To avoid this allocation, reading one byte at time, in this way end of stream
         * will be reached before memory saturation.
         */
        for (std::size_t i = 0; i < lengthFromBuffer; i++) {
            uint8_t byte;
            mBufferReader.read(byte);
            valueBuffer.push_back(byte);
        }
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Unable to read tlv: " + std::string(e.what()));
    }

    // Is Tag in the dictionary ?
    TlvWrapperInterface *tlvWrapper =
        mResponseHandler.getTlvDictionary().getTlvWrapperForTag(tagFromBuffer);
    if (tlvWrapper == nullptr) {

        // Tag is unknown
        throw Exception("Cannot parse unknown tag " + std::to_string(tagFromBuffer));
    }

    // Read value
    try {
        tlvWrapper->readFrom(valueBuffer);
    } catch (TlvWrapperInterface::Exception &e) {
        throw Exception("Error reading value for tag " + std::to_string(tagFromBuffer) + ": " +
                        std::string(e.what()));
    }

    return true;
}
}
}