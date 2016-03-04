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
