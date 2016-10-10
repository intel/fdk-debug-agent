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

#include "Tlv/TlvWrapperInterface.hpp"
#include "Util/ByteStreamReader.hpp"
#include <algorithm>
#include <vector>

namespace debug_agent
{
namespace tlv
{
/**
 * Real implementation of a TlvWrapperInterface for a Vector of value as TLV Value.
 * The validity of a shadow variable managed by a TlvVectorWrapper is intrinsic: unless the vector
 * size is null, the vector is valid.
 * @tparam ValueType the type of the values managed by this TlvVectorWrapper
 * @see TlvWrapperInterface
 */
template <typename ValueType>
class TlvVectorWrapper final : public TlvWrapperInterface
{
public:
    /**
     * @param[in] values the reference to the shadow runtime variable
     */
    TlvVectorWrapper(std::vector<ValueType> &values) : mValues(values) { invalidate(); }

    void readFrom(const util::Buffer &binarySource) override
    {
        mValues.clear();
        try {
            util::MemoryByteStreamReader reader(binarySource);
            while (!reader.isEOS()) {
                ValueType value;
                reader.read(value);
                mValues.push_back(value);
            }
        } catch (util::ByteStreamReader::Exception &e) {
            throw Exception("Can not read array element: " + std::string(e.what()));
        }
    }

    virtual void invalidate() noexcept override { mValues.clear(); }

private:
    std::vector<ValueType> &mValues;
};
}
}
