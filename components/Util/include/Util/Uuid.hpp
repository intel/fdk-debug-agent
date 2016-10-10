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

#include <inttypes.h>
#include <string>
#include <cstring>

namespace debug_agent
{
namespace util
{

/** UUID based on the microsoft implementation of the rfc4122
 * @see https://tools.ietf.org/html/rfc4122
 *
 * The binary encoding of the microsoft implementation differs slightly from the rfc4122:
 * Bits Bytes   Name    Microsoft endianness      rfc4122 endianness
 * 32   4       Data1   Native                    Big
 * 16   2       Data2   Native                    Big
 * 16   2       Data3   Native                    Big
 * 64   8       Data4   Big                       Big
 *
 * @see https://en.wikipedia.org/wiki/Globally_unique_identifier#Binary_encoding
 */
struct Uuid
{
    uint32_t data1;   /* Native endianness, i.e. little endian on Intel platforms */
    uint16_t data2;   /* Native endianness */
    uint16_t data3;   /* Native endianness */
    uint8_t data4[8]; /* Big endian */

    /* Convert to string */
    std::string toString() const noexcept;

    bool operator==(Uuid &other) const
    {
        return data1 == other.data1 && data2 == other.data2 && data3 == other.data3 &&
               std::memcmp(&data4, &other.data4, sizeof(data4)) == 0;
    }

    bool operator!=(Uuid &other) const noexcept { return !(*this == other); }

    /** Convert from an other uuid type. It must have the same size. */
    template <typename UuidType>
    void fromOtherUuidType(const UuidType &other)
    {
        static_assert(sizeof(UuidType) == sizeof(Uuid), "Wrong uuid type size");

        const Uuid *otherAsUuid = reinterpret_cast<const Uuid *>(&other);
        *this = *otherAsUuid;
    }

    /** Convert to an other uuid type. It must have the same size. */
    template <typename UuidType>
    void toOtherUuidType(UuidType &other) const noexcept
    {
        static_assert(sizeof(UuidType) == sizeof(Uuid), "Wrong uuid type size");

        Uuid *otherAsUuid = reinterpret_cast<Uuid *>(&other);
        *otherAsUuid = *this;
    }
};
}
}
