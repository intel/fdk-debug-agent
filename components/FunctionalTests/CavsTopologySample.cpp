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
#include <algorithm>

using namespace debug_agent::cavs;
using namespace debug_agent::util;

namespace debug_agent {

const dsp_fw::AudioDataFormatIpc audioFormat = {
    dsp_fw::SamplingFrequency::FS_48000HZ,
    dsp_fw::BitDepth::DEPTH_16BIT,
    0,
    dsp_fw::ChannelConfig::CHANNEL_CONFIG_STEREO,
    dsp_fw::InterleavingStyle::CHANNELS_BLOCKS_INTERLEAVING,
    1,
    0,
    dsp_fw::SampleType::SIGNED_INTEGER,
    0
};

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

const std::vector<std::string> moduleNames = {
    "copier",
    "aec",
    "gain",
    "ns",
    "mixin",
    "src",
    "mixout"
};

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
DSPinListInfo newEmptyPinList()
{
    return DSPinListInfo();
}

/** Helper function to create pin list */
DSPinListInfo newPinList(const std::vector<uint32_t> queueIds)
{
    DSPinListInfo info {};
    for (auto queueId : queueIds) {
        dsp_fw::PinProps props {};
        props.format = audioFormat;
        props.stream_type = dsp_fw::STREAM_TYPE_PCM;
        props.phys_queue_id = queueId;

        info.pin_info.push_back(props);
    }
    return info;
}

/** Helper function to create module instance */
DSModuleInstanceProps newModuleInstance(uint32_t type, uint32_t instance,
    const DSPinListInfo &inputs, const DSPinListInfo &outputs,
    const dsp_fw::ConnectorNodeId &inputGateway =
        dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId),
    const dsp_fw::ConnectorNodeId &outputGateway =
        dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId))
{
    DSModuleInstanceProps props {};
    props.id = Topology::joinModuleInstanceId(type, instance);
    props.input_gateway = inputGateway;
    props.output_gateway = outputGateway;
    props.input_pins = inputs;
    props.output_pins = outputs;
    return props;
}

/** Helper function to create gateway */
dsp_fw::GatewayProps newGateway(const dsp_fw::ConnectorNodeId &connectorId)
{
    dsp_fw::GatewayProps p {};
    p.attribs = 0;
    p.id = connectorId.val.dw;
    return p;
}

/** Helper function to create task */
DSTaskProps newTask(uint32_t id, const std::vector<uint32_t> &ids)
{
    DSTaskProps props {};
    props.task_id = id;
    props.module_instance_id = ids;
    return props;
}

/** Helper function to create scheduler */
DSSchedulersInfo newScheduler(const std::vector<DSTaskProps> &tasks)
{
    DSSchedulerProps props {};
    props.core_id = 0;
    props.task_info = tasks;

    DSSchedulersInfo info;
    info.scheduler_info.push_back(props);

    return info;
}

/** Helper function to create pipeline */
DSPplProps newPipeline(uint32_t id, const std::vector<uint32_t> &instanceIds,
    const std::vector<uint32_t> &taskIds)
{
    DSPplProps props {};
    props.id = id;
    props.module_instances = instanceIds;
    props.ll_tasks = taskIds;
    return props;
}

void CavsTopologySample::createFirwareObjects(
    std::vector<DSModuleInstanceProps> &moduleInstances,
    std::vector<dsp_fw::GatewayProps> &gateways,
    uint32_t &maxPplCount,
    std::vector<uint32_t> &pipelineIds,
    std::vector<DSPplProps> &pipelines,
    std::vector<DSSchedulersInfo> &schedulers,
    std::vector<ModuleEntry> &modules,
    std::vector<char> &fwConfig,
    std::vector<char> &hwConfig)
{
    /* Filling module entries */
    uint32_t i = 0;
    for (auto &moduleName: moduleNames)
    {
        ModuleEntry entry;
        StringHelper::setStringToFixedSizeArray(entry.name, sizeof(entry.name), moduleName);
        for (uint32_t &intValue : entry.uuid) {
            /* filling four bytes with i value */
            intValue = (i << 24) | (i << 16) | (i << 8) | i;
        }

        modules.push_back(entry);
        ++i;
    }

    /* Filling module instances */
    moduleInstances = {
        /* Pipe 1 */
        newModuleInstance(module_copier, 1,
            newEmptyPinList(),
            newPinList({ queue_pipe1 }),
            dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostInputClass, 1)),

        newModuleInstance(module_aec, 2,
            newPinList({ queue_pipe1 }),
            newPinList({ queue_pipe1 })),

        newModuleInstance(module_gain, 5,
            newPinList({ queue_pipe1 }),
            newPinList({ queue_pipe1_3 })),

        /* Pipe 2*/
        newModuleInstance(module_gain, 4,
            newEmptyPinList(),
            newPinList({ queue_pipe2 }),
            dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostInputClass, 2)),

        newModuleInstance(module_aec, 5,
            newPinList({ queue_pipe2 }),
            newPinList({ queue_pipe2 })),

        newModuleInstance(module_ns, 6,
            newPinList({ queue_pipe2 }),
            newPinList({ queue_pipe2_3 })),

        /* Pipe 3*/
        newModuleInstance(module_mixin, 1,
            newPinList({ queue_pipe1_3, queue_pipe2_3 }),
            newPinList({ queue_pipe3 })),

        newModuleInstance(module_src, 0,
            newPinList({ queue_pipe3 }),
            newPinList({ queue_pipe3 })),

        newModuleInstance(module_gain, 9,
            newPinList({ queue_pipe3, queue_pipe3_4 }),
            newEmptyPinList(),
            dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId),
            dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaLinkOutputClass, 1)),

        /* Pipe4 */
        newModuleInstance(module_gain, 1,
            newPinList({}),
            newPinList({ queue_pipe4, queue_pipe3_4 }),
            dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kDmicLinkInputClass, 1)),

        newModuleInstance(module_ns, 2,
            newPinList({ queue_pipe4 }),
            newPinList({ queue_pipe4 })),

        newModuleInstance(module_mixout, 3,
            newPinList({ queue_pipe4 }),
            newEmptyPinList(),
            dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kInvalidNodeId),
            dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostOutputClass, 1)),
    };

    /* Module instances ioctls are ordered by the module instace id, so sorting the array
     * to reflect the same order */
    std::sort(moduleInstances.begin(), moduleInstances.end(),
        [](const DSModuleInstanceProps &p1, const DSModuleInstanceProps& p2) -> bool
        {
            return p1.id < p2.id;
        }
    );

    /* Filling gateways */
    gateways = {
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostInputClass, 1)),
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostInputClass, 2)),
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaLinkOutputClass, 1)),
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kDmicLinkInputClass, 1)),
        newGateway(dsp_fw::ConnectorNodeId(dsp_fw::ConnectorNodeId::kHdaHostOutputClass, 1))
    };

    /* Filling schedulers */
    schedulers = {
        newScheduler({
            newTask(1, {
                Topology::joinModuleInstanceId(module_copier, 1)
            }),
            newTask(2, {
                Topology::joinModuleInstanceId(module_aec, 2),
                Topology::joinModuleInstanceId(module_gain, 5)
            }),
            newTask(3, {
                Topology::joinModuleInstanceId(module_gain, 4)
            }),
            newTask(9, {
                Topology::joinModuleInstanceId(module_aec, 5),
                Topology::joinModuleInstanceId(module_ns, 6)
            }),
            newTask(4, {
                Topology::joinModuleInstanceId(module_mixin, 1),
                Topology::joinModuleInstanceId(module_src, 0),
                Topology::joinModuleInstanceId(module_gain, 9)
            }),
            newTask(5, {
                Topology::joinModuleInstanceId(module_gain, 1)
            }),
            newTask(6, {
                Topology::joinModuleInstanceId(module_ns, 2),
                Topology::joinModuleInstanceId(module_mixout, 3)
            }),
        })
    };

    /* Filling pipelines */
    maxPplCount = 10;
    pipelineIds = { 1, 2, 3, 4 };
    pipelines = {
        newPipeline(1, {
            Topology::joinModuleInstanceId(module_copier, 1),
            Topology::joinModuleInstanceId(module_aec, 2),
            Topology::joinModuleInstanceId(module_gain, 5),
        },
        { 1, 2 }),
        newPipeline(2, {
            Topology::joinModuleInstanceId(module_gain, 4),
            Topology::joinModuleInstanceId(module_aec, 5),
            Topology::joinModuleInstanceId(module_ns, 6)
        },
        { 3, 9 }),
        newPipeline(3, {
            Topology::joinModuleInstanceId(module_mixin, 1),
            Topology::joinModuleInstanceId(module_src, 0),
            Topology::joinModuleInstanceId(module_gain, 9)
        },
        { 4 }),
        newPipeline(4, {
            Topology::joinModuleInstanceId(module_gain, 1),
            Topology::joinModuleInstanceId(module_ns, 2),
            Topology::joinModuleInstanceId(module_mixout, 3)
        },
        { 5, 6 }),
    };

    /* Filling firmware config */
    fwConfig = {
        /* Tag for FW_VERSION: 0x00000000 */
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
        static_cast<char>(moduleNames.size()), 0x00, 0x00, 0x00,

        /* Tag for MAX_PPL_COUNT : 9 */
        9, 0x00, 0x00, 0x00,
        /* Length = 4 bytes */
        0x04, 0x00, 0x00, 0x00,
        /* Value : 2 */
        static_cast<char>(maxPplCount), 0x00, 0x00, 0x00
    };

    /* Filling hardware config */
    hwConfig = {
        /* Tag for DSP_CORES: 0x00000001 */
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
        static_cast<char>(gateways.size()), 0x00, 0x00, 0x00
    };
}

}