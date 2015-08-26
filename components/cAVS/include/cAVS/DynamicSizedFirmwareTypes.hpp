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
 */

namespace debug_agent
{
namespace cavs
{

/* All firmware array sizes are stored on 32 bits unsigned integer*/
using ArraySizeType = uint32_t;

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

}
}
