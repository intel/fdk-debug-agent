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
#include "cAVS/DspFw/AudioFormat.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/EnumHelper.hpp"
#include "Util/StructureChangeTracking.hpp"
#include <inttypes.h>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* Cannot track changes of this type because it is not in the public fw headers, only in private
 * ones. */
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
        kHdaHostInoutClass = 2, // for future use
        kHdaLinkOutputClass = 8,
        kHdaLinkInputClass = 9,
        kHdaLinkInoutClass = 10, // for future use
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
        static const util::EnumHelper<ConnectorNodeId::Type> helper(
            {{ConnectorNodeId::kHdaHostOutputClass, "hda-host-out-gateway"},
             {ConnectorNodeId::kHdaHostInputClass, "hda-host-in-gateway"},
             {ConnectorNodeId::kHdaHostInoutClass, "hda-host-inout-gateway"},
             {ConnectorNodeId::kHdaLinkOutputClass, "hda-link-out-gateway"},
             {ConnectorNodeId::kHdaLinkInputClass, "hda-link-in-gateway"},
             {ConnectorNodeId::kHdaLinkInoutClass, "hda-link-inout-gateway"},
             {ConnectorNodeId::kDmicLinkInputClass, "dmic-link-in-gateway"},
             {ConnectorNodeId::kI2sLinkOutputClass, "i2s-link-out-gateway"},
             {ConnectorNodeId::kI2sLinkInputClass, "i2s-link-in-gateway"},
             {ConnectorNodeId::kSlimbusLinkOutputClass, "slimbus-link-out-gateway"},
             {ConnectorNodeId::kSlimbusLinkInputClass, "slimbus-link-in-gateway"},
             {ConnectorNodeId::kALHLinkOutputClass, "alh-link-out-gateway"},
             {ConnectorNodeId::kALHLinkInputClass, "alh-link-in-gateway"}});

        return helper;
    };

    /**
    * Creates the connector node id from dma type and virtual dma index.
    * @param dma_type Type of DMA Connector.
    * @param v_index Virtual DMA Index.
    */
    ConnectorNodeId(Type dma_type, uint8_t v_index) { Init(dma_type, v_index); }

    explicit ConnectorNodeId(uint32_t node_id) { val.dw = node_id; }
    static const uint32_t kInvalidNodeId = 0xffffffff;

    ConnectorNodeId() { val.dw = kInvalidNodeId; }

    void Init(Type dma_type, uint8_t v_index)
    {
        val.dw = 0;
        val.f.dma_type = dma_type;
        val.f.v_index = v_index;
    }

    inline uint32_t GetBareNodeId() const { return val.dw; }

    bool operator==(const ConnectorNodeId &other) const { return val.dw == other.val.dw; }

    void fromStream(util::ByteStreamReader &reader) { reader.read(val.dw); }

    void toStream(util::ByteStreamWriter &writer) const { writer.write(val.dw); }
};

/* PinProps */

CHECK_SIZE(private_fw::dsp_fw::PinProps, 32);
CHECK_MEMBER(private_fw::dsp_fw::PinProps, stream_type, 0, StreamType);
CHECK_MEMBER(private_fw::dsp_fw::PinProps, format, 4, private_fw::dsp_fw::AudioDataFormatIpc);
CHECK_MEMBER(private_fw::dsp_fw::PinProps, phys_queue_id, 28, uint32_t);

struct PinProps
{
    StreamType stream_type;
    AudioDataFormatIpc format;
    uint32_t phys_queue_id;

    static const uint32_t invalidQueueId = 0xFFFFFFFF;

    bool operator==(const PinProps &other) const
    {
        return phys_queue_id == other.phys_queue_id && format == other.format &&
               stream_type == other.stream_type;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(stream_type);
        reader.read(format);
        reader.read(phys_queue_id);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(stream_type);
        writer.write(format);
        writer.write(phys_queue_id);
    }
};

/* PinListInfo */
CHECK_SIZE(private_fw::dsp_fw::PinListInfo, 36);
CHECK_MEMBER(private_fw::dsp_fw::PinListInfo, pin_count, 0, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::PinListInfo, pin_info, 4, private_fw::dsp_fw::PinProps[1]);

struct PinListInfo
{
    std::vector<PinProps> pin_info;

    bool operator==(const PinListInfo &other) const { return pin_info == other.pin_info; }

    void fromStream(util::ByteStreamReader &reader) { reader.readVector<ArraySizeType>(pin_info); }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVector<ArraySizeType>(pin_info);
    }
};

/* ModuleInstanceProps */
CHECK_SIZE(private_fw::dsp_fw::ModuleInstanceProps, 124);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, id, 0, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, dp_queue_type, 4, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, queue_alignment, 8, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, cp_usage_mask, 12, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, stack_bytes, 16, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, bss_total_bytes, 20, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, bss_used_bytes, 24, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, ibs_bytes, 28, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, obs_bytes, 32, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, cpc, 36, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, cpc_peak, 40, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, input_queues, 44,
             private_fw::dsp_fw::PinListInfo);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, output_queues, 80,
             private_fw::dsp_fw::PinListInfo);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, input_gateway, 116, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::ModuleInstanceProps, output_gateway, 120, uint32_t);

struct ModuleInstanceProps
{
    CompoundModuleId id;
    uint32_t dp_queue_type;
    uint32_t queue_alignment;
    uint32_t cp_usage_mask;
    uint32_t stack_bytes;
    uint32_t bss_total_bytes;
    uint32_t bss_used_bytes;
    uint32_t ibs_bytes;
    uint32_t obs_bytes;
    uint32_t cpc;
    uint32_t cpc_peak;
    PinListInfo input_pins;
    PinListInfo output_pins;
    ConnectorNodeId input_gateway;
    ConnectorNodeId output_gateway;

    bool operator==(const ModuleInstanceProps &other) const
    {
        return id == other.id && dp_queue_type == other.dp_queue_type &&
               queue_alignment == other.queue_alignment && cp_usage_mask == other.cp_usage_mask &&
               stack_bytes == other.stack_bytes && bss_total_bytes == other.bss_total_bytes &&
               bss_used_bytes == other.bss_used_bytes && ibs_bytes == other.ibs_bytes &&
               obs_bytes == other.obs_bytes && cpc == other.cpc && cpc_peak == other.cpc_peak &&
               input_pins == other.input_pins && output_pins == other.output_pins &&
               input_gateway == other.input_gateway && output_gateway == other.output_gateway;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(id);
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
        reader.read(input_pins);
        reader.read(output_pins);
        reader.read(input_gateway);
        reader.read(output_gateway);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(id);
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
        writer.write(input_pins);
        writer.write(output_pins);
        writer.write(input_gateway);
        writer.write(output_gateway);
    }
};
}
}
}
