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
#include <iostream>
#include <string>

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
namespace dsp_fw
{

/* IMPORTANT: this header is a mess and will be split in a subsequent patch. */

/* All firmware array sizes are stored on 32 bits unsigned integer*/
using ArraySizeType = uint32_t;

struct CompoundModuleId
{
    uint16_t moduleId;
    uint16_t instanceId;

    bool operator ==(const CompoundModuleId &other) const
    {
        return moduleId == other.moduleId && instanceId == other.instanceId;
    }

    /* Required because this class is used as key of std::map */
    bool operator<(const CompoundModuleId& other) const
    {
        return toInt(*this) < toInt(other);
    }

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

private:
    static inline uint32_t toInt(const CompoundModuleId &compId)
    {
        return (static_cast<uint32_t>(compId.moduleId) << 16) | compId.instanceId;
    }
};

static std::ostream& operator<< (std::ostream& stream, const CompoundModuleId& id)
{
    stream << id.toString();
    return stream;
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
    std::vector<CompoundModuleId> module_instances;
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
        reader.readVectorAndRecurse<ArraySizeType>(module_instances);
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
        writer.writeVectorAndRecurse<ArraySizeType>(module_instances);
        writer.writeVector<ArraySizeType>(ll_tasks);
        writer.writeVector<ArraySizeType>(dp_tasks);
    }
};

struct DSTaskProps
{
    uint32_t  task_id;
    std::vector<CompoundModuleId>  module_instance_id;

    bool operator ==(const DSTaskProps &other) const
    {
        return task_id == other.task_id &&
            module_instance_id == other.module_instance_id;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(task_id);
        reader.readVectorAndRecurse<ArraySizeType>(module_instance_id);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(task_id);
        writer.writeVectorAndRecurse<ArraySizeType>(module_instance_id);
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
        return pin_info == other.pin_info;
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
        reader.read(input_gateway);
        reader.read(output_gateway);
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
        writer.write(input_gateway);
        writer.write(output_gateway);
    }
};

struct PipelinesListInfo
{
    using PipeLineIdType = uint32_t;
    std::vector<PipeLineIdType> ppl_id;

    static std::size_t getAllocationSize(std::size_t count)
    {
        return sizeof(ArraySizeType)+ count * sizeof(PipeLineIdType);
    }

    bool operator == (const PipelinesListInfo& other) const
    {
        return ppl_id == other.ppl_id;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVector<ArraySizeType>(ppl_id);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVector<ArraySizeType>(ppl_id);
    }
};


}
}
}
