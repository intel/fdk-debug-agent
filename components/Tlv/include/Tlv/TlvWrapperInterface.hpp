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
#include "Util/Buffer.hpp"
#include <cstddef>
#include <stdexcept>

namespace debug_agent
{
namespace tlv
{
/**
 * Defines the requirement for a TlvWrapper.
 * A TlvWrapper is specialized for a given TLV Value Type using a template class parameter.
 * The TlvWrapperInterface allows to abstract this specialization in order to have polymorphic
 * collection of TlvWrapper.
 * A TlvWrapper is the link between the TLV in a binary form from a binary TLV list buffer and
 * its shadow representation in a runtime variable.
 * When a binary TLV list is read by the TlvUnpack, it uses the associated TlvWrapper in order
 * to reflect the binary value from the buffer to the shadow runtime variable.
 * The wrapper also provides a way to invalidate the shadow variable.
 * @see TlvWrapper
 * @see TlvUnpack
 */
class TlvWrapperInterface
{
public:
    struct Exception final : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /**
     * Read the value from the binary source provided and reflect it to the shadow runtime variable.
     * The caller must ensure there are enough chars from binarySource to read the complete value.
     * @param[in] binarySource the memory buffer from which the value will be read
     * @throw TlvWrapperInterface::Exception
     */
    virtual void readFrom(const util::Buffer &binarySource) = 0;

    /**
     * Invalidate the shadow variable.
     */
    virtual void invalidate() noexcept = 0;

    virtual ~TlvWrapperInterface() {}
};
}
}
