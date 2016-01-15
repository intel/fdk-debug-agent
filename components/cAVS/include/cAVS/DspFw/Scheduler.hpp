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
struct CoreIdTrait
{
    using RawType = uint32_t;
};
}
using CoreId = util::WrappedRaw<detail::CoreIdTrait>;

/* TaskProps */
CHECK_SIZE(private_fw::TaskProps, 12);
CHECK_MEMBER(private_fw::TaskProps, task_id, 0, uint32_t);
CHECK_MEMBER(private_fw::TaskProps, module_instance_count, 4, uint32_t);
CHECK_MEMBER(private_fw::TaskProps, module_instance_id, 8, uint32_t[1]);

struct TaskProps
{
    uint32_t task_id;
    std::vector<CompoundModuleId> module_instance_id;

    bool operator==(const TaskProps &other) const
    {
        return task_id == other.task_id && module_instance_id == other.module_instance_id;
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

/* SchedulerProps */
CHECK_SIZE(private_fw::SchedulerProps, 24);
CHECK_MEMBER(private_fw::SchedulerProps, processing_domain, 0, uint32_t);
CHECK_MEMBER(private_fw::SchedulerProps, core_id, 4, uint32_t);
CHECK_MEMBER(private_fw::SchedulerProps, task_count, 8, uint32_t);
CHECK_MEMBER(private_fw::SchedulerProps, task_info, 12, private_fw::TaskProps[1]);

struct SchedulerProps
{
    uint32_t processing_domain;
    uint32_t core_id;
    std::vector<TaskProps> task_info;

    bool operator==(const SchedulerProps &other) const
    {
        return processing_domain == other.processing_domain && core_id == other.core_id &&
               task_info == other.task_info;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(processing_domain);
        reader.read(core_id);
        reader.readVector<ArraySizeType>(task_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(processing_domain);
        writer.write(core_id);
        writer.writeVector<ArraySizeType>(task_info);
    }
};

/* SchedulersInfo */

CHECK_SIZE(private_fw::SchedulersInfo, 28);
CHECK_MEMBER(private_fw::SchedulersInfo, scheduler_count, 0, uint32_t);
CHECK_MEMBER(private_fw::SchedulersInfo, scheduler_info, 4, private_fw::SchedulerProps[1]);

struct SchedulersInfo
{
    std::vector<SchedulerProps> scheduler_info;

    bool operator==(const SchedulersInfo &other) const
    {
        return scheduler_info == other.scheduler_info;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVector<ArraySizeType>(scheduler_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVector<ArraySizeType>(scheduler_info);
    }
};
}
}
}
