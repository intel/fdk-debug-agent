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

#include "CavsTopologySample.hpp"
#include "cAVS/Topology.hpp"
#include "Util/StringHelper.hpp"
#include "Util/AssertAlways.hpp"
#include "Util/EnumHelper.hpp"
#include <algorithm>

using namespace debug_agent::cavs;
using namespace debug_agent::util;

namespace debug_agent
{

const dsp_fw::AudioDataFormatIpc audioFormat = {
    dsp_fw::SamplingFrequency::FS_48000HZ,
    dsp_fw::BitDepth::DEPTH_16BIT,
    0,
    dsp_fw::ChannelConfig::CHANNEL_CONFIG_STEREO,
    dsp_fw::InterleavingStyle::CHANNELS_BLOCKS_INTERLEAVING,
    1,
    0,
    dsp_fw::SampleType::SIGNED_INTEGER,
    0};

enum Modules : uint16_t
{
    module_copier = 0,
    module_aec = 1,
    module_gain = 5,
    module_ns = 9,
    module_mixin = 1024,
    module_src = 4012,
    module_mixout = 4100
};

const util::EnumHelper<Modules> moduleHelper({
    {Modules::module_copier, "copier"},
    {Modules::module_aec, "aec"},
    {Modules::module_gain, "gain"},
    {Modules::module_ns, "ns"},
    {Modules::module_mixin, "mixin"},
    {Modules::module_src, "src"},
    {Modules::module_mixout, "mixout"},
});

enum Queue
{
    queue_pipe1,
    queue_pipe2,
    queue_pipe3,
    queue_pipe4,
    queue_pipe1_3,
    queue_pipe2_3,
    queue_pipe3_4,
};

/** Helper function to create empty pin list */
dsp_fw::PinListInfo newEmptyPinList()
{
    return dsp_fw::PinListInfo();
}

/** Helper function to create pin list */
dsp_fw::PinListInfo newPinList(const std::vector<Queue> queueIds)
{
    dsp_fw::PinListInfo info;
    for (auto queueId : queueIds) {
        dsp_fw::PinProps props;
        props.format = audioFormat;
        props.stream_type = dsp_fw::StreamType::ePcm;
        props.phys_queue_id = queueId;

        info.pin_info.push_back(props);
    }
    return info;
}

/** Helper function to create module instance */
dsp_fw::ModuleInstanceProps newModuleInstance(
    Modules type, uint16_t instance, const dsp_fw::PinListInfo &inputs,
    const dsp_fw::PinListInfo &outputs,
    const dsp_fw::ConnectorNodeId &inputGateway =
        dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId),
    const dsp_fw::ConnectorNodeId &outputGateway =
        dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId))
{
    dsp_fw::ModuleInstanceProps props;
    props.id = {type, instance};
    props.dp_queue_type = 0;
    props.queue_alignment = 0;
    props.cp_usage_mask = 0;
    props.stack_bytes = 0;
    props.bss_total_bytes = 0;
    props.bss_used_bytes = 0;
    props.ibs_bytes = 0;
    props.obs_bytes = 0;
    props.cpc = 0;
    props.cpc_peak = 0;
    props.input_gateway = inputGateway;
    props.output_gateway = outputGateway;
    props.input_pins = inputs;
    props.output_pins = outputs;
    return props;
}

/** Helper function to create gateway */
dsp_fw::GatewayProps newGateway(const dsp_fw::ConnectorNodeId &connectorId)
{
    dsp_fw::GatewayProps p;
    p.attribs.dw = 0;
    p.id = connectorId.val.dw;
    return p;
}

/** Helper function to create task */
dsp_fw::TaskProps newTask(uint32_t id, const std::vector<dsp_fw::CompoundModuleId> &ids)
{
    dsp_fw::TaskProps props;
    props.task_id = id;
    props.module_instance_id = ids;
    return props;
}

/** Helper function to create scheduler */
dsp_fw::SchedulersInfo newScheduler(const std::vector<dsp_fw::TaskProps> &tasks)
{
    dsp_fw::SchedulerProps props;
    props.processing_domain = 0;
    props.core_id = 0;
    props.task_info = tasks;

    dsp_fw::SchedulersInfo info;
    info.scheduler_info.push_back(props);

    return info;
}

/** Helper function to create pipeline */
dsp_fw::PplProps newPipeline(dsp_fw::PipeLineIdType id, uint32_t priority,
                             const std::vector<dsp_fw::CompoundModuleId> &instanceIds,
                             const std::vector<uint32_t> &taskIds)
{
    dsp_fw::PplProps props;
    props.id = id;
    props.priority = priority;
    props.state = 0;
    props.total_memory_bytes = 0;
    props.used_memory_bytes = 0;
    props.context_pages = 0;
    props.module_instances = instanceIds;
    props.ll_tasks = taskIds;
    return props;
}

const size_t CavsTopologySample::maxPplCount = 10;
const size_t CavsTopologySample::gatewaysCount = 5;

void CavsTopologySample::createInstanceFirmwareObjects(
    std::vector<dsp_fw::ModuleInstanceProps> &moduleInstances,
    std::vector<dsp_fw::GatewayProps> &gateways, std::vector<dsp_fw::PipeLineIdType> &pipelineIds,
    std::vector<dsp_fw::PplProps> &pipelines, std::vector<dsp_fw::SchedulersInfo> &schedulers)
{
    /* Filling module instances */
    moduleInstances = {
        /* Pipe 1 */
        newModuleInstance(module_copier, 1, newEmptyPinList(), newPinList({queue_pipe1}),
                          dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostInputClass, 1)),

        newModuleInstance(module_aec, 2, newPinList({queue_pipe1}), newPinList({queue_pipe1})),

        newModuleInstance(module_gain, 5, newPinList({queue_pipe1}), newPinList({queue_pipe1_3})),

        /* Pipe 2*/
        newModuleInstance(module_gain, 4, newEmptyPinList(), newPinList({queue_pipe2}),
                          dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostInputClass, 2)),

        newModuleInstance(module_aec, 5, newPinList({queue_pipe2}), newPinList({queue_pipe2})),

        newModuleInstance(module_ns, 6, newPinList({queue_pipe2}), newPinList({queue_pipe2_3})),

        /* Pipe 3*/
        newModuleInstance(module_mixin, 1, newPinList({queue_pipe1_3, queue_pipe2_3}),
                          newPinList({queue_pipe3})),

        newModuleInstance(module_src, 0, newPinList({queue_pipe3}), newPinList({queue_pipe3})),

        newModuleInstance(module_gain, 9, newPinList({queue_pipe3, queue_pipe3_4}),
                          newEmptyPinList(),
                          dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId),
                          dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaLinkOutputClass, 1)),

        /* Pipe4 */
        newModuleInstance(module_gain, 1, newPinList({}), newPinList({queue_pipe3_4, queue_pipe4}),
                          dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kDmicLinkInputClass, 1)),

        newModuleInstance(module_ns, 2, newPinList({queue_pipe4}), newPinList({queue_pipe4})),

        newModuleInstance(module_mixout, 3, newPinList({queue_pipe4}), newEmptyPinList(),
                          dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId),
                          dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostOutputClass, 1)),
    };

    /* Module instances ioctls are ordered by the module instace id, so sorting the array
     * to reflect the same order */
    std::sort(moduleInstances.begin(), moduleInstances.end(),
              [](const dsp_fw::ModuleInstanceProps &p1,
                 const dsp_fw::ModuleInstanceProps &p2) -> bool { return p1.id < p2.id; });

    /* Filling gateways */
    gateways = {
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostInputClass, 1)),
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostInputClass, 2)),
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaLinkOutputClass, 1)),
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kDmicLinkInputClass, 1)),
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostOutputClass, 1))};
    ASSERT_ALWAYS(gateways.size() == gatewaysCount);

    /* Filling schedulers */
    schedulers = {newScheduler({
        newTask(1,
                {
                    {module_copier, 1},
                }),
        newTask(2,
                {
                    {module_aec, 2}, {module_gain, 5},
                }),
        newTask(3,
                {
                    {module_gain, 4},
                }),
        newTask(9,
                {
                    {module_aec, 5}, {module_ns, 6},
                }),
        newTask(4,
                {
                    {module_mixin, 1}, {module_src, 0}, {module_gain, 9},
                }),
        newTask(5,
                {
                    {module_gain, 1},
                }),
        newTask(6,
                {
                    {module_ns, 2}, {module_mixout, 3},
                }),
    })};

    /* Filling pipelines
     * The priority of each pipeline is set wisely in order to check the debug agent
     * will order them by priority: from highest to lowest. Hence, pipeline are provided to
     * the Debug Agent in this order: ID4;ID2;ID1;ID3 and the Debug Agent shall order them
     * in this order: ID1;ID2;ID3;ID4 according chosen priorities.
     */
    using ID = dsp_fw::PipeLineIdType;
    pipelineIds = {ID{4}, ID{2}, ID{1}, ID{3}};
    pipelines = {newPipeline(ID{4}, 40,
                             {
                                 {module_gain, 1}, {module_ns, 2}, {module_mixout, 3},
                             },
                             {5, 6}),
                 newPipeline(ID{2}, 20,
                             {
                                 {module_gain, 4}, {module_aec, 5}, {module_ns, 6},
                             },
                             {3, 9}),
                 newPipeline(ID{1}, 10,
                             {
                                 {module_copier, 1}, {module_aec, 2}, {module_gain, 5},
                             },
                             {1, 2}),
                 newPipeline(ID{3}, 30,
                             {
                                 {module_mixin, 1}, {module_src, 0}, {module_gain, 9},
                             },
                             {4})};
}

void CavsTopologySample::createFirmwareObjects(std::vector<dsp_fw::ModuleEntry> &modules,
                                               Buffer &fwConfig, Buffer &hwConfig)
{
    /* Filling module entries */
    for (auto &moduleEntry : moduleHelper.getEnumToStringMap()) {
        dsp_fw::ModuleEntry entry;
        StringHelper::setStringToFixedSizeArray(entry.name, sizeof(entry.name), moduleEntry.second);

        for (uint32_t &intValue : entry.uuid) {
            /* using module id as uuid parts */
            intValue = moduleEntry.first;
        }
        entry.module_id = moduleEntry.first;

        entry.state_flags = 0;
        entry.type.ul = 0;
        entry.hash.fill(0);
        entry.entry_point = 0;
        entry.cfg_offset = 0;
        entry.cfg_count = 0;
        entry.affinity_mask = 0;
        entry.instance_max_count = 0;
        entry.instance_stack_size = 0;

        for (auto &segment : entry.segments) {
            segment.flags.ul = 0;
            segment.v_base_addr = 0;
            segment.file_offset = 0;
        }

        modules.push_back(entry);
    }

    /* Filling firmware config */
    fwConfig = {/* Tag for FW_VERSION: 0x00000000 */
                0x00, 0x00, 0x00, 0x00,
                /* Length = 8 bytes */
                0x08, 0x00, 0x00, 0x00,
                /* Value */
                /* major and minor */
                0x01, 0x00, 0x02, 0x00,
                /* hot fix and build */
                0x03, 0x00, 0x04, 0x00,

                /* Tag for MODULES_COUNT : 12 */
                12, 0x00, 0x00, 0x00,
                /* Length = 4 bytes */
                0x04, 0x00, 0x00, 0x00,
                /* Value */
                static_cast<uint8_t>(moduleHelper.getEnumToStringMap().size()), 0x00, 0x00, 0x00,

                /* Tag for MAX_PPL_COUNT : 9 */
                9, 0x00, 0x00, 0x00,
                /* Length = 4 bytes */
                0x04, 0x00, 0x00, 0x00,
                /* Value */
                static_cast<uint8_t>(maxPplCount), 0x00, 0x00, 0x00,

                /* Tag for MAX_MOD_INST_COUNT : 13 */
                0x0d, 0x00, 0x00, 0x00,
                /* Length = 4 bytes */
                0x04, 0x00, 0x00, 0x00,
                /* Value */
                uint8_t{maxModInstCount}, 0x00, 0x00, 0x00};

    /* Filling hardware config */
    hwConfig = {/* Tag for DSP_CORES: 0x00000001 */
                0x01, 0x00, 0x00, 0x00,
                /* Length = 4 bytes */
                0x04, 0x00, 0x00, 0x00,
                /* Value: nb core */
                uint8_t{dspCoreCount}, 0x00, 0x00, 0x00,

                /* Tag for GATEWAY_COUNT : 6 */
                0x06, 0x00, 0x00, 0x00,
                /* Length = 4 bytes */
                0x4, 0x00, 0x00, 0x00,
                /* Value */
                static_cast<uint8_t>(gatewaysCount), 0x00, 0x00, 0x00};
}
}
