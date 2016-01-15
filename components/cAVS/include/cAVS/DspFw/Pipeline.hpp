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
#include "Util/WrappedRaw.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/StructureChangeTracking.hpp"

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

namespace detail
{
struct PipeLineIdTrait
{
    using RawType = uint32_t;
};
}
using PipeLineIdType = util::WrappedRaw<detail::PipeLineIdTrait>;

/* PipelinesListInfo */

CHECK_SIZE(private_fw::PipelinesListInfo, 8);
CHECK_MEMBER(private_fw::PipelinesListInfo, ppl_count, 0, uint32_t);
CHECK_MEMBER(private_fw::PipelinesListInfo, ppl_id, 4, uint32_t[1]);

struct PipelinesListInfo
{
    std::vector<PipeLineIdType> ppl_id;

    static std::size_t getAllocationSize(std::size_t count)
    {
        return sizeof(ArraySizeType) + count * sizeof(PipeLineIdType);
    }

    bool operator==(const PipelinesListInfo &other) const { return ppl_id == other.ppl_id; }

    void fromStream(util::ByteStreamReader &reader) { reader.readVector<ArraySizeType>(ppl_id); }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVector<ArraySizeType>(ppl_id);
    }
};

/* ModInstListInfo */
CHECK_SIZE(private_fw::ModInstListInfo, 8);
CHECK_MEMBER(private_fw::ModInstListInfo, module_instance_count, 0, uint32_t);
CHECK_MEMBER(private_fw::ModInstListInfo, module_instance_id, 4, uint32_t[1]);

using ModInstListInfo = std::vector<CompoundModuleId>;

/* TaskListInfo */
CHECK_SIZE(private_fw::TaskListInfo, 8);
CHECK_MEMBER(private_fw::TaskListInfo, task_id_count, 0, uint32_t);
CHECK_MEMBER(private_fw::TaskListInfo, task_id, 4, uint32_t[1]);

using TaskListInfo = std::vector<uint32_t>;

/* PplProps */
CHECK_SIZE(private_fw::PplProps, 48);
CHECK_MEMBER(private_fw::PplProps, id, 0, uint32_t);
CHECK_MEMBER(private_fw::PplProps, priority, 4, uint32_t);
CHECK_MEMBER(private_fw::PplProps, state, 8, uint32_t);
CHECK_MEMBER(private_fw::PplProps, total_memory_bytes, 12, uint32_t);
CHECK_MEMBER(private_fw::PplProps, used_memory_bytes, 16, uint32_t);
CHECK_MEMBER(private_fw::PplProps, context_pages, 20, uint32_t);
CHECK_MEMBER(private_fw::PplProps, module_instances, 24, private_fw::ModInstListInfo);
CHECK_MEMBER(private_fw::PplProps, ll_tasks, 32, private_fw::TaskListInfo);
CHECK_MEMBER(private_fw::PplProps, dp_tasks, 40, private_fw::TaskListInfo);

struct PplProps
{
    PipeLineIdType id;
    uint32_t priority;
    uint32_t state;
    uint32_t total_memory_bytes;
    uint32_t used_memory_bytes;
    uint32_t context_pages;
    ModInstListInfo module_instances;
    TaskListInfo ll_tasks;
    TaskListInfo dp_tasks;

    bool operator==(const PplProps &other) const
    {
        return id == other.id && priority == other.priority && state == other.state &&
               total_memory_bytes == other.total_memory_bytes &&
               used_memory_bytes == other.used_memory_bytes &&
               context_pages == other.context_pages && module_instances == other.module_instances &&
               ll_tasks == other.ll_tasks && ll_tasks == other.ll_tasks;
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
}
}
}
