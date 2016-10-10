/*
 * Copyright (c) 2016, Intel Corporation
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
CHECK_SIZE(private_fw::dsp_fw::PerfDataItem, 16);
CHECK_MEMBER(private_fw::dsp_fw::PerfDataItem, resource_id, 0, uint32_t);
// skip power_mode, rsvd and is_removed as they are bit-fields
CHECK_MEMBER(private_fw::dsp_fw::PerfDataItem, peak_kcps, 8, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::PerfDataItem, avg_kcps, 12, uint32_t);
struct PerfDataItem
{
    /**
     * ID of the resource running the load being reported.
     * if moduleId = 0, it is a core.
     * */
    CompoundModuleId resourceId;

    union
    {
        uint32_t full;
        struct
        {
            uint32_t powerMode : 1; //< 0: D0; 1: D0i3
            uint32_t reserved : 30;
            uint32_t isRemoved : 1;
        } bits;
    } details;

    /** Peak KCPS (Kilo Cycles Per Second) captured. */
    uint32_t peak;
    /** Average KCPS (Kilo Cycles Per Second) measured. */
    uint32_t average;

    PerfDataItem(uint16_t moduleId, uint16_t instanceId, bool powerMode, bool isRemoved,
                 uint32_t _peak, uint32_t _average)
    {
        resourceId.moduleId = moduleId;
        resourceId.instanceId = instanceId;
        details.bits.powerMode = (powerMode ? 1 : 0);
        details.bits.reserved = 0;
        details.bits.isRemoved = (isRemoved ? 1 : 0);
        peak = _peak;
        average = _average;
    }

    PerfDataItem() : PerfDataItem(0, 0, false, false, 0, 0) {}

    bool operator==(const PerfDataItem &other) const
    {
        return resourceId == other.resourceId && details.full == other.details.full &&
               peak == other.peak && average == other.average;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(resourceId);
        reader.read(details.full);
        reader.read(peak);
        reader.read(average);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(resourceId);
        writer.write(details.full);
        writer.write(peak);
        writer.write(average);
    }
};

/* GatewaysInfo */
CHECK_SIZE(private_fw::dsp_fw::GlobalPerfData, 20);
CHECK_MEMBER(private_fw::dsp_fw::GlobalPerfData, perf_item_count, 0, uint32_t);
CHECK_MEMBER(private_fw::dsp_fw::GlobalPerfData, perf_items, 4,
             private_fw::dsp_fw::PerfDataItem[1]);

struct GlobalPerfData
{
    std::vector<PerfDataItem> items;

    static std::size_t getAllocationSize(std::size_t count)
    {
        return sizeof(ArraySizeType) + count * sizeof(PerfDataItem);
    }

    bool operator==(const GlobalPerfData &other) const { return items == other.items; }

    void fromStream(util::ByteStreamReader &reader) { reader.readVector<ArraySizeType>(items); }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVector<ArraySizeType>(items);
    }
};
}
}
}
