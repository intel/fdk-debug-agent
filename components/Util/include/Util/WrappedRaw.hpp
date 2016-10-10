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

#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"

namespace debug_agent
{
namespace util
{

/** Class to implement a strong typedef over basic types.
 *
 * It is used to compile type guard against mixing different semantic values
 * represented by the same basic type.
 *
 * This class is intended to wrap basic types, but it may also be used to wrap
 * any complex type implementing the IsCompoundSerializableType concept.
 * Mind the absence of an `operator->` though.
 *
 * @tparam Trait A unique class that so that `WrappedRaw<Trait>` will be a
 *               different type unique to this wrapping.
 *               `Trait::RawType` must be the type to wrapp.
 * @code
 * // Example: Strong type frequency to differentiate them at compile type from period
 * struct StrongFrequencyTrait { using RawType = uint32_t; };
 * using StrongFrequency = WrappedRaw<StrongFrequencyTrait>;
 * @endcode
 */
template <class Trait>
class WrappedRaw final
{
public:
    using RawType = typename Trait::RawType;

    explicit WrappedRaw(RawType value) : mValue(value) {}
    /** Default initialization is needed for deserialization. */
    explicit WrappedRaw() : mValue{} {}

    bool operator==(const WrappedRaw left) const { return mValue == left.mValue; }
    bool operator<(const WrappedRaw left) const { return mValue < left.mValue; }

    RawType getValue() const { return mValue; };

    void fromStream(ByteStreamReader &reader) { reader.read(mValue); }
    void toStream(ByteStreamWriter &writer) const { writer.write(mValue); }

private:
    RawType mValue;
};

} // namespace util
} // namespace debug_agent
