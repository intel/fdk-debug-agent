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

#include "CavsTopologySample.hpp"
#include "cAVS/Topology.hpp"
#include "Util/StringHelper.hpp"
#include "Util/AssertAlways.hpp"
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

enum Modules
{
    module_copier,
    module_aec,
    module_gain,
    module_ns,
    module_mixin,
    module_src,
    module_mixout
};

const std::vector<std::string> moduleNames = {"copier", "aec", "gain",  "ns",
                                              "mixin",  "src", "mixout"};

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
dsp_fw::PinListInfo newPinList(const std::vector<uint32_t> queueIds)
{
    dsp_fw::PinListInfo info{};
    for (auto queueId : queueIds) {
        dsp_fw::PinProps props{};
        props.format = audioFormat;
        props.stream_type = dsp_fw::StreamType::STREAM_TYPE_PCM;
        props.phys_queue_id = queueId;

        info.pin_info.push_back(props);
    }
    return info;
}

/** Helper function to create module instance */
dsp_fw::ModuleInstanceProps newModuleInstance(
    uint32_t type, uint32_t instance, const dsp_fw::PinListInfo &inputs,
    const dsp_fw::PinListInfo &outputs,
    const dsp_fw::ConnectorNodeId &inputGateway =
        dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId),
    const dsp_fw::ConnectorNodeId &outputGateway =
        dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId))
{
    dsp_fw::ModuleInstanceProps props{};
    props.id.moduleId = type;
    props.id.instanceId = instance;
    props.input_gateway = inputGateway;
    props.output_gateway = outputGateway;
    props.input_pins = inputs;
    props.output_pins = outputs;
    return props;
}

/** Helper function to create gateway */
dsp_fw::GatewayProps newGateway(const dsp_fw::ConnectorNodeId &connectorId)
{
    dsp_fw::GatewayProps p{};
    p.attribs = 0;
    p.id = connectorId.val.dw;
    return p;
}

/** Helper function to create task */
dsp_fw::TaskProps newTask(uint32_t id, const std::vector<dsp_fw::CompoundModuleId> &ids)
{
    dsp_fw::TaskProps props{};
    props.task_id = id;
    props.module_instance_id = ids;
    return props;
}

/** Helper function to create scheduler */
dsp_fw::SchedulersInfo newScheduler(const std::vector<dsp_fw::TaskProps> &tasks)
{
    dsp_fw::SchedulerProps props{};
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
    dsp_fw::PplProps props{};
    props.id = id;
    props.priority = priority;
    props.module_instances = instanceIds;
    props.ll_tasks = taskIds;
    return props;
}

const size_t CavsTopologySample::moduleCount = 7;
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
    ASSERT_ALWAYS(moduleCount == moduleNames.size());
    uint32_t i = 0;
    for (auto &moduleName : moduleNames) {
        dsp_fw::ModuleEntry entry{};
        StringHelper::setStringToFixedSizeArray(entry.name, sizeof(entry.name), moduleName);
        for (uint32_t &intValue : entry.uuid) {
            /* filling four bytes with i value */
            intValue = (i << 24) | (i << 16) | (i << 8) | i;
        }
        entry.module_id = i;

        modules.push_back(entry);
        ++i;
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
                static_cast<char>(moduleCount), 0x00, 0x00, 0x00,

                /* Tag for MAX_PPL_COUNT : 9 */
                9, 0x00, 0x00, 0x00,
                /* Length = 4 bytes */
                0x04, 0x00, 0x00, 0x00,
                /* Value */
                static_cast<char>(maxPplCount), 0x00, 0x00, 0x00};

    /* Filling hardware config */
    hwConfig = {/* Tag for DSP_CORES: 0x00000001 */
                0x01, 0x00, 0x00, 0x00,
                /* Length = 4 bytes */
                0x04, 0x00, 0x00, 0x00,
                /* Value: nb core */
                0x01, 0x00, 0x00, 0x00,

                /* Tag for GATEWAY_COUNT : 6 */
                0x06, 0x00, 0x00, 0x00,
                /* Length = 4 bytes */
                0x4, 0x00, 0x00, 0x00,
                /* Value */
                static_cast<char>(gatewaysCount), 0x00, 0x00, 0x00};
}
}
