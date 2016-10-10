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
#include "Util/StructureChangeTracking.hpp"

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* FwVersion */

CHECK_SIZE(private_fw::FwVersion, 8);
CHECK_MEMBER(private_fw::FwVersion, major, 0, uint16_t);
CHECK_MEMBER(private_fw::FwVersion, minor, 2, uint16_t);
CHECK_MEMBER(private_fw::FwVersion, hotfix, 4, uint16_t);
CHECK_MEMBER(private_fw::FwVersion, build, 6, uint16_t);

struct FwVersion
{
    uint16_t major;
    uint16_t minor;
    uint16_t hotfix;
    uint16_t build;

    bool operator==(const FwVersion &other)
    {
        return major == other.major && minor == other.minor && hotfix == other.hotfix &&
               build == other.build;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(major);
        reader.read(minor);
        reader.read(hotfix);
        reader.read(build);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(major);
        writer.write(minor);
        writer.write(hotfix);
        writer.write(build);
    }
};

/* DmaBufferConfig */

CHECK_SIZE(private_fw::DmaBufferConfig, 8);
CHECK_MEMBER(private_fw::DmaBufferConfig, min_size_bytes, 0, uint32_t);
CHECK_MEMBER(private_fw::DmaBufferConfig, max_size_bytes, 4, uint32_t);

struct DmaBufferConfig
{
    uint32_t min_size_bytes;
    uint32_t max_size_bytes;

    bool operator==(const DmaBufferConfig &other)
    {
        return min_size_bytes == other.min_size_bytes && max_size_bytes == other.max_size_bytes;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(min_size_bytes);
        reader.read(max_size_bytes);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(min_size_bytes);
        writer.write(max_size_bytes);
    }
};

/* Importing FwConfigParams enum */
using FwConfigParams = private_fw::FwConfigParams;

/* Importing HwConfigParams enum */
using HwConfigParams = private_fw::HwConfigParams;

/* Importing BaseModuleParams enum */
using BaseModuleParams = private_fw::dsp_fw::BaseModuleParams;
}
}
}
