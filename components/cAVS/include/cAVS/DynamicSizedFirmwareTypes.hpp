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

#include "cAVS/FirmwareTypes.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include <vector>
#include <stdexcept>
#include <limits>
#include <assert.h>

/** Some firmware structures have dynamic size and can't be fit to a C structure:
 * - PplProps
 * - TaskProps
 * - SchedulerProps
 * - SchedulersInfo
 * - ModuleInstanceProps
 * - PinListInfo
 *
 * Therefore these structures are unusable.
 *
 * This header introduces new versions of these structures using c++ vectors. Then they can
 * be serialized to a binary stream.
 *
 * All the new structure versions are prefixed with "DS", meaning "Dynamic Sized":
 * - DSPplProps
 * - DSTaskProps
 * - DSSchedulerPropsreadVector<ArraySizeType>
 * - DSSchedulersInfo
 * - DSModuleInstanceProps
 * - DSPinListInfo
 */

namespace debug_agent
{
namespace cavs
{

/* All firmware array sizes are stored on 32 bits unsigned integer*/
using ArraySizeType = uint32_t;

/* Operator == for fw types */

static bool operator == (const dsp_fw::AudioDataFormatIpc &f1, const dsp_fw::AudioDataFormatIpc &f2)
{
    return f1.bit_depth == f2.bit_depth &&
        f1.channel_config == f2.channel_config &&
        f1.channel_map == f2.channel_map &&
        f1.interleaving_style == f2.interleaving_style &&
        f1.number_of_channels == f2.number_of_channels &&
        f1.reserved == f2.reserved &&
        f1.sample_type == f2.sample_type &&
        f1.sampling_frequency == f2.sampling_frequency &&
        f1.valid_bit_depth == f2.valid_bit_depth;
}

static bool operator == (const dsp_fw::PinProps &p1, const dsp_fw::PinProps &p2)
{
    return p1.phys_queue_id == p2.phys_queue_id &&
        p1.format == p2.format &&
        p1.stream_type == p2.stream_type;
}

static bool operator == (const dsp_fw::ConnectorNodeId &c1, const dsp_fw::ConnectorNodeId c2)
{
    return c1.val.dw == c2.val.dw;
}

/* Dynamic-sized types */

struct DSPplProps
{
    uint32_t        id;
    uint32_t        priority;
    uint32_t        state;
    uint32_t        total_memory_bytes;
    uint32_t        used_memory_bytes;
    uint32_t        context_pages;
    std::vector<uint32_t> module_instances;
    std::vector<uint32_t> ll_tasks;
    std::vector<uint32_t> dp_tasks;

    bool operator ==(const DSPplProps &other) const
    {
        return id == other.id &&
            priority == other.priority &&
            state == other.state &&
            total_memory_bytes == other.total_memory_bytes &&
            used_memory_bytes == other.used_memory_bytes &&
            context_pages == other.context_pages &&
            module_instances == other.module_instances &&
            ll_tasks == other.ll_tasks &&
            ll_tasks == other.ll_tasks;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(id);
        reader.read(priority);
        reader.read(state);
        reader.read(total_memory_bytes);
        reader.read(used_memory_bytes);
        reader.read(context_pages);
        reader.readVector<ArraySizeType>(module_instances);
        reader.readVector<ArraySizeType>(ll_tasks);
        reader.readVector<ArraySizeType>(dp_tasks);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(id);
        writer.write(priority);
        writer.write(state);
        writer.write(total_memory_bytes);
        writer.write(used_memory_bytes);
        writer.write(context_pages);
        writer.writeVector<ArraySizeType>(module_instances);
        writer.writeVector<ArraySizeType>(ll_tasks);
        writer.writeVector<ArraySizeType>(dp_tasks);
    }
};

struct DSTaskProps
{
    uint32_t  task_id;
    std::vector<uint32_t>  module_instance_id;

    bool operator ==(const DSTaskProps &other) const
    {
        return task_id == other.task_id &&
            module_instance_id == other.module_instance_id;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(task_id);
        reader.readVector<ArraySizeType>(module_instance_id);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(task_id);
        writer.writeVector<ArraySizeType>(module_instance_id);
    }
};

struct DSSchedulerProps
{
    uint32_t   processing_domain;
    uint32_t   core_id;
    std::vector<DSTaskProps>  task_info;

    bool operator ==(const DSSchedulerProps &other) const
    {
        return processing_domain == other.processing_domain &&
            core_id == other.core_id &&
            task_info == other.task_info;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(processing_domain);
        reader.read(core_id);
        reader.readVectorAndRecurse<ArraySizeType>(task_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(processing_domain);
        writer.write(core_id);
        writer.writeVectorAndRecurse<ArraySizeType>(task_info);
    }
};

struct DSSchedulersInfo
{
    std::vector<DSSchedulerProps>  scheduler_info;

    bool operator ==(const DSSchedulersInfo &other) const
    {
        return scheduler_info == other.scheduler_info;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVectorAndRecurse<ArraySizeType>(scheduler_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVectorAndRecurse<ArraySizeType>(scheduler_info);
    }
};

struct DSPinListInfo
{
    std::vector<dsp_fw::PinProps> pin_info;

    bool operator == (const DSPinListInfo& other) const
    {
        if (pin_info.size() != other.pin_info.size()) {
            return false;
        }
        for (std::size_t i = 0; i < pin_info.size(); i++) {
            if (!(pin_info[i] == other.pin_info[i])) {
                return false;
            }
        }
        return true;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVector<ArraySizeType>(pin_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVector<ArraySizeType>(pin_info);
    }
};


struct DSModuleInstanceProps
{
    uint32_t          id;
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
    DSPinListInfo       input_pins;
    DSPinListInfo       output_pins;
    dsp_fw::ConnectorNodeId   input_gateway;
    dsp_fw::ConnectorNodeId   output_gateway;

    bool operator == (const DSModuleInstanceProps& other) const
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
        input_pins.fromStream(reader);
        output_pins.fromStream(reader);
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
        input_pins.toStream(writer);
        output_pins.toStream(writer);
        writer.write(input_gateway);
        writer.write(output_gateway);
    }
};


}
}
