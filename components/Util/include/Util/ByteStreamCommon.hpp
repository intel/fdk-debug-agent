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

#include <type_traits>
#include <cstdint>

namespace debug_agent
{
namespace util
{

/** The "value" member of this structure is true if the supplied type should be serialized using a
 * simple memory copy.
 *
 * Currently "simple serializable" types are integral types.
 */
template <typename T>
struct IsSimpleSerializableType
{
    static const bool value = std::is_integral<T>::value;
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

/** The "value" member  of this structure is true if the supplied type is composite and should be
 * serialized using it implicit toStream() and fromStream() interface, because simple memory copy
 * doesn't work in this case.
 */
template <typename T>
struct IsCompoundSerializableType
{
    static const bool value =
        !IsSimpleSerializableType<T>::value && !IsEnumSerializableType<T>::value;
};
}
}
