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
