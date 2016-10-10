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
