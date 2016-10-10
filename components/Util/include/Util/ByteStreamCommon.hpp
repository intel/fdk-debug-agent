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

#include <type_traits>
#include <array>
#include <cstdint>

namespace debug_agent
{
namespace util
{

/** The "value" member of this structure is true if the supplied type should be serialized using a
 * simple memory copy.
 *
 * Currently "simple serializable" types are integral types and pointers.
 */
template <typename T>
struct IsSimpleSerializableType
{
    static const bool value = std::is_integral<T>::value || std::is_pointer<T>::value;
};

/** Enums are encoded on 32 bits */
using EnumEncodingType = uint32_t;

/** The "value" member of this structure is true if the supplied type is an enum and should be
 * serialized using the type "EnumEncodingType"
 */
template <typename T>
struct IsEnumSerializableType
{
    static const bool value = std::is_enum<T>::value;
};

template <typename...>
struct IsFixedArray : std::false_type
{
};

template <typename T, std::size_t N>
struct IsFixedArray<std::array<T, N>> : std::true_type
{
};

template <typename T, std::size_t N>
struct IsFixedArray<T[N]> : std::integral_constant<bool, N != 0>
{
};

/** The "value" member of this structure is true if the supplied type is a
 * C array (with a fixed size) or an std::array and should be serialized using iterative
 * serialization.
 */
template <typename T>
struct IsArraySerializableType
{
    static const bool value = IsFixedArray<T>::value;
};

/** The "value" member  of this structure is true if the supplied type is composite and should be
 * serialized using it implicit toStream() and fromStream() interface, because simple memory copy
 * doesn't work in this case.
 */
template <typename T>
struct IsCompoundSerializableType
{
    static const bool value = !IsSimpleSerializableType<T>::value &&
                              !IsEnumSerializableType<T>::value &&
                              !IsArraySerializableType<T>::value;
};
}
}
