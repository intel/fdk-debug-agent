/*
 * Copyright (c) 2015-2016, Intel Corporation
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

#include "cAVS/DspFw/ExternalFirmwareHeaders.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/WrappedRaw.hpp"
#include "Util/StructureChangeTracking.hpp"
#include <inttypes.h>
#include <iostream>
#include <string>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* All firmware array sizes are stored on 32 bits unsigned integer*/
using ArraySizeType = uint32_t;

namespace detail
{
/** Unique trait to for parameterId. */
struct ParameterIdTrait
{
    using RawType = uint32_t;
};

} // namespace detail

using ParameterId = util::WrappedRaw<detail::ParameterIdTrait>;

/** By convention the base firmware module id is 0 */
static const uint16_t baseFirmwareModuleId = 0;

/** By convention the base firmware instance id is 0 */
static const uint16_t baseFirmwareInstanceId = 0;

/* This compound module id contains a type id and an instance id */
struct CompoundModuleId
{
    uint16_t moduleId;
    uint16_t instanceId;

    bool operator==(const CompoundModuleId &other) const
    {
        return moduleId == other.moduleId && instanceId == other.instanceId;
    }

    /* Required because this class is used as key of std::map */
    bool operator<(const CompoundModuleId &other) const { return toInt() < other.toInt(); }

    void fromStream(util::ByteStreamReader &reader)
    {
        /* In the firmware structure the compound id is store on an uint32_t:
        * - 16 left bits -> module id
        * - 16 right bits-> instance id
        * Therefore the memory mapping is:
        * - first 16 bits: instance id
        * - next 16 bits: module id
        */
        reader.read(instanceId);
        reader.read(moduleId);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(instanceId);
        writer.write(moduleId);
    }

    std::string toString() const
    {
        return "(" + std::to_string(moduleId) + "," + std::to_string(instanceId) + ")";
    }

    constexpr uint32_t toInt() const
    {
        return (static_cast<uint32_t>(moduleId) << 16) | instanceId;
    }
};

inline static std::ostream &operator<<(std::ostream &stream, const CompoundModuleId &id)
{
    stream << id.toString();
    return stream;
}
}
}
}
