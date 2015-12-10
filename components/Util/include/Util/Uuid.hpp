/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
*   The source code contained  or  described herein and all documents related to
*   the source code ("Material") are owned by Intel Corporation or its suppliers
*   or licensors.  Title to the  Material remains with  Intel Corporation or its
*   suppliers and licensors. The Material contains trade secrets and proprietary
*   and  confidential  information of  Intel or its suppliers and licensors. The
*   Material  is  protected  by  worldwide  copyright  and trade secret laws and
*   treaty  provisions. No part of the Material may be used, copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way without Intel's prior express written permission.
*   No license  under any  patent, copyright, trade secret or other intellectual
*   property right is granted to or conferred upon you by disclosure or delivery
*   of the Materials,  either expressly, by implication, inducement, estoppel or
*   otherwise.  Any  license  under  such  intellectual property  rights must be
*   express and approved by Intel in writing.
*
********************************************************************************
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
    std::string toString() const NOEXCEPT;

    bool operator==(Uuid &other) const
    {
        return data1 == other.data1 && data2 == other.data2 && data3 == other.data3 &&
               std::memcmp(&data4, &other.data4, sizeof(data4)) == 0;
    }

    bool operator!=(Uuid &other) const NOEXCEPT { return !(*this == other); }

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
    void toOtherUuidType(UuidType &other) const NOEXCEPT
    {
        static_assert(sizeof(UuidType) == sizeof(Uuid), "Wrong uuid type size");

        Uuid *otherAsUuid = reinterpret_cast<Uuid *>(&other);
        *otherAsUuid = *this;
    }
};
}
}
