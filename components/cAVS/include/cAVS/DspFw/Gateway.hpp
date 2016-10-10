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

#include "cAVS/DspFw/Common.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/StructureChangeTracking.hpp"

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/** GatewayAttributes */

CHECK_SIZE(private_fw::GatewayAttributes, 4);
CHECK_MEMBER(private_fw::GatewayAttributes, dw, 0, uint32_t);

union GatewayAttributes
{
    uint32_t dw;
    struct
    {
        uint32_t lp_buffer_alloc : 1;
        uint32_t _rsvd : 31;
    } bits;

    bool operator==(const GatewayAttributes &other) const { return dw == other.dw; }

    bool operator!=(const GatewayAttributes &other) const { return !(*this == other); }

    void fromStream(util::ByteStreamReader &reader) { reader.read(dw); }

    void toStream(util::ByteStreamWriter &writer) const { writer.write(dw); }
};

/* GatewayProps */
CHECK_SIZE(private_fw::GatewayProps, 8);
CHECK_MEMBER(private_fw::GatewayProps, id, 0, uint32_t);
CHECK_MEMBER(private_fw::GatewayProps, attribs, 4, private_fw::GatewayAttributes);

struct GatewayProps
{
    /**
    * Gateway ID (refer to ConnectorNodeId).
    */
    uint32_t id;
    /**
    * Gateway attributes (refer to GatewayAttributes).
    */
    GatewayAttributes attribs;

    bool operator==(const GatewayProps &other) const
    {
        return id == other.id && attribs == other.attribs;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(id);
        reader.read(attribs);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(id);
        writer.write(attribs);
    }
};

/* GatewaysInfo */
CHECK_SIZE(private_fw::GatewaysInfo, 12);
CHECK_MEMBER(private_fw::GatewaysInfo, gateway_count, 0, uint32_t);
CHECK_MEMBER(private_fw::GatewaysInfo, gateways, 4, private_fw::GatewayProps[1]);

struct GatewaysInfo
{
    std::vector<GatewayProps> gateways;

    static std::size_t getAllocationSize(std::size_t count)
    {
        return sizeof(ArraySizeType) + count * sizeof(GatewayProps);
    }

    bool operator==(const GatewaysInfo &other) const { return gateways == other.gateways; }

    void fromStream(util::ByteStreamReader &reader) { reader.readVector<ArraySizeType>(gateways); }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVector<ArraySizeType>(gateways);
    }
};
}
}
}
