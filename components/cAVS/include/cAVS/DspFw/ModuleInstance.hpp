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
#include "cAVS/DspFw/AudioFormat.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/EnumHelper.hpp"
#include <inttypes.h>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* This type exists in the fw header but is not easily includable, so copying it */
struct ConnectorNodeId
{
    union
    {
        uint32_t dw;
        struct
        {
            uint32_t v_index : 8;
            uint32_t dma_type : 5;
            uint32_t _rsvd : 19;
        } f;
    } val;

    enum Type
    {
        kHdaHostOutputClass = 0,
        kHdaHostInputClass = 1,
        kHdaHostInoutClass = 2,   // for future use
        kHdaLinkOutputClass = 8,
        kHdaLinkInputClass = 9,
        kHdaLinkInoutClass = 10,  // for future use
        kDmicLinkInputClass = 11,
        kI2sLinkOutputClass = 12,
        kI2sLinkInputClass = 13,
        kSlimbusLinkOutputClass = 14,
        kSlimbusLinkInputClass = 15,
        kALHLinkOutputClass = 16,
        kALHLinkInputClass = 17
    };

    static const util::EnumHelper<ConnectorNodeId::Type> &getTypeEnumHelper()
    {
        static const util::EnumHelper<ConnectorNodeId::Type> helper({
            { ConnectorNodeId::kHdaHostOutputClass, "hda-host-out-gateway" },
            { ConnectorNodeId::kHdaHostInputClass, "hda-host-in-gateway" },
            { ConnectorNodeId::kHdaHostInoutClass, "hda-host-inout-gateway" },
            { ConnectorNodeId::kHdaLinkOutputClass, "hda-link-out-gateway" },
            { ConnectorNodeId::kHdaLinkInputClass, "hda-link-in-gateway" },
            { ConnectorNodeId::kHdaLinkInoutClass, "hda-link-inout-gateway" },
            { ConnectorNodeId::kDmicLinkInputClass, "dmic-link-in-gateway" },
            { ConnectorNodeId::kI2sLinkOutputClass, "i2s-link-out-gateway" },
            { ConnectorNodeId::kI2sLinkInputClass, "i2s-link-in-gateway" },
            { ConnectorNodeId::kSlimbusLinkOutputClass, "slimbus-link-out-gateway" },
            { ConnectorNodeId::kSlimbusLinkInputClass, "slimbus-link-in-gateway" },
            { ConnectorNodeId::kALHLinkOutputClass, "alh-link-out-gateway" },
            { ConnectorNodeId::kALHLinkInputClass, "alh-link-in-gateway" }
        });

        return helper;
    };

    /**
    * Creates the connector node id from dma type and virtual dma index.
    * @param dma_type Type of DMA Connector.
    * @param v_index Virtual DMA Index.
    */
    ConnectorNodeId(Type dma_type, uint8_t v_index)
    {
        Init(dma_type, v_index);
    }

    explicit ConnectorNodeId(uint32_t node_id)
    {
        val.dw = node_id;
    }
    static const uint32_t kInvalidNodeId = 0xffffffff;

    ConnectorNodeId()
    {
        val.dw = kInvalidNodeId;
    }

    void Init(Type dma_type, uint8_t v_index)
    {
        val.dw = 0;
        val.f.dma_type = dma_type;
        val.f.v_index = v_index;
    }

    inline uint32_t GetBareNodeId() const { return val.dw; }

    bool operator == (const ConnectorNodeId &other) const
    {
        return val.dw == other.val.dw;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(val.dw);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(val.dw);
    }
};
static_assert(sizeof(ConnectorNodeId) == 4, "Wrong ConnectorNodeId size");

/** This type does not exist in the fw yet */
struct PinProps
{
    StreamType          stream_type;
    AudioDataFormatIpc  format;
    uint32_t            phys_queue_id;

    static const uint32_t invalidQueueId = 0xFFFFFFFF;

    bool operator == (const PinProps &other) const
    {
        return phys_queue_id == other.phys_queue_id &&
            format == other.format &&
            stream_type == other.stream_type;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(stream_type);
        format.fromStream(reader);
        reader.read(phys_queue_id);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(stream_type);
        format.toStream(writer);
        writer.write(phys_queue_id);
    }
};
static_assert(sizeof(PinProps) == 32, "Wrong PinProps size");

struct PinListInfo
{
    std::vector<PinProps> pin_info;

    bool operator == (const PinListInfo& other) const
    {
        return pin_info == other.pin_info;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVectorAndRecurse<ArraySizeType>(pin_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVectorAndRecurse<ArraySizeType>(pin_info);
    }
};


struct ModuleInstanceProps
{
    CompoundModuleId  id;
    uint32_t          dp_queue_type;
    uint32_t          queue_alignment;
    uint32_t          cp_usage_mask;
    uint32_t          stack_bytes;
    uint32_t          bss_total_bytes;
    uint32_t          bss_used_bytes;
    uint32_t          ibs_bytes;
    uint32_t          obs_bytes;
    uint32_t          cpc;
    uint32_t          cpc_peak;
    PinListInfo       input_pins;
    PinListInfo       output_pins;
    ConnectorNodeId   input_gateway;
    ConnectorNodeId   output_gateway;

    bool operator == (const ModuleInstanceProps& other) const
    {
        return
            id == other.id &&
            dp_queue_type == other.dp_queue_type &&
            queue_alignment == other.queue_alignment &&
            cp_usage_mask == other.cp_usage_mask &&
            stack_bytes == other.stack_bytes &&
            bss_total_bytes == other.bss_total_bytes &&
            bss_used_bytes == other.bss_used_bytes &&
            ibs_bytes == other.ibs_bytes &&
            obs_bytes == other.obs_bytes &&
            cpc == other.cpc &&
            cpc_peak == other.cpc_peak &&
            input_pins == other.input_pins &&
            output_pins == other.output_pins &&
            input_gateway == other.input_gateway &&
            output_gateway == other.output_gateway;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        id.fromStream(reader);
        reader.read(dp_queue_type);
        reader.read(queue_alignment);
        reader.read(cp_usage_mask);
        reader.read(stack_bytes);
        reader.read(bss_total_bytes);
        reader.read(bss_used_bytes);
        reader.read(ibs_bytes);
        reader.read(obs_bytes);
        reader.read(cpc);
        reader.read(cpc_peak);
        input_pins.fromStream(reader);
        output_pins.fromStream(reader);
        input_gateway.fromStream(reader);
        output_gateway.fromStream(reader);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        id.toStream(writer);
        writer.write(dp_queue_type);
        writer.write(queue_alignment);
        writer.write(cp_usage_mask);
        writer.write(stack_bytes);
        writer.write(bss_total_bytes);
        writer.write(bss_used_bytes);
        writer.write(ibs_bytes);
        writer.write(obs_bytes);
        writer.write(cpc);
        writer.write(cpc_peak);
        input_pins.toStream(writer);
        output_pins.toStream(writer);
        input_gateway.toStream(writer);
        output_gateway.toStream(writer);
    }
};


}
}
}
