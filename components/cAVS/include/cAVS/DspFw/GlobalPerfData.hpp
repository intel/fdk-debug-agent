/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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
