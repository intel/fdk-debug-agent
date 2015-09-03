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
#include "Core/InstanceModelConverter.hpp"
#include "Util/StringHelper.hpp"

using namespace debug_agent::cavs;
using namespace debug_agent::ifdk_objects::instance;

namespace debug_agent
{
namespace core
{

std::shared_ptr<InstanceModel> InstanceModelConverter::createModel()
{
    try
    {
        mSystem.getTopology(mTopology);
    }
    catch (cavs::System::Exception &e)
    {
        throw Exception("Cannot get topology from fw: " + std::string(e.what()));
    }

    auto subsystemCollection = new SubsystemCollection();
    subsystemCollection->add(createSubsystem());

    InstanceModel::CollectionMap collectionMap;
    collectionMap[subsystemName] = std::shared_ptr<BaseCollection>(subsystemCollection);

    return std::shared_ptr<InstanceModel>(
        new InstanceModel(createSystem(), collectionMap));
}

std::shared_ptr<ifdk_objects::instance::System> InstanceModelConverter::createSystem()
{
    auto system = std::shared_ptr<ifdk_objects::instance::System>(
        new ifdk_objects::instance::System());

    system->setTypeName(systemName);
    system->setInstanceId(systemId);

    auto coll = new SubsystemRefCollection(collectionName_subsystem);
    coll->add(SubsystemRef(subsystemName, subsystemId));
    system->getChildren().add(coll);

    return system;
}

std::shared_ptr<Subsystem> InstanceModelConverter::createSubsystem()
{
    auto subsystem = std::shared_ptr<Subsystem>(new Subsystem());
    subsystem->setTypeName(subsystemName);
    subsystem->setInstanceId(subsystemId);

    Links &links = subsystem->getLinks();

    /* Parents */
    subsystem->getParents().add(new SystemRef(systemName, systemId));

    /* Children*/
    Children &children = subsystem->getChildren();

    /* Pipes */
    auto pipeCollection = new InstanceRefCollection(collectionName_pipe);
    for (auto &pipeline : mTopology.pipelines) {
        pipeCollection->add(InstanceRef(typeName_pipe, std::to_string(pipeline.id)));
    }
    children.add(pipeCollection);

    /* Cores */
    auto coreCollection = new InstanceRefCollection(collectionName_core);
    if (!mSystem.getHwConfig().isDspCoreCountValid) {
        throw Exception("Core count is invalid.");
    }
    for (uint32_t coreId = 0; coreId < mSystem.getHwConfig().dspCoreCount; coreId++) {
        coreCollection->add(InstanceRef(typeName_core, std::to_string(coreId)));
    }
    children.add(coreCollection);

    /* Tasks */
    auto taskCollection = new InstanceRefCollection(collectionName_task);
    for (auto &schedulersInfo : mTopology.schedulers) {
        for (auto &scheduler : schedulersInfo.scheduler_info) {
            for (auto &task : scheduler.task_info) {
                taskCollection->add(
                    InstanceRef(typeName_task, std::to_string(task.task_id)));
            }
        }
    }
    children.add(taskCollection);

    /* Gateways */
    auto gatewayCollection = new ComponentRefCollection(collectionName_gateway);
    for (auto &gateway : mTopology.gateways) {
        dsp_fw::ConnectorNodeId connector(gateway.id);

        gatewayCollection->add(ComponentRef(findGatewayTypeName(connector),
            std::to_string(findGatewayInstanceId(connector))));
    }
    children.add(gatewayCollection);

    /* Modules and links to gateway*/
    auto moduleCollection = new ComponentRefCollection(collectionName_module);
    for (auto &moduleEntry : mTopology.moduleInstances) {

        auto &module = moduleEntry.second;
        uint16_t moduleId, instanceId;
        Topology::splitModuleInstanceId(module.id, moduleId, instanceId);

        /* Finding the module type to get the type name */
        auto moduleName = findModuleEntryName(moduleId);

        moduleCollection->add(ComponentRef(moduleName, std::to_string(instanceId)));

        if (module.input_gateway.val.dw != dsp_fw::ConnectorNodeId::kInvalidNodeId) {
            /* Connected to an input gateway */

            Link l(
                From(
                    findGatewayTypeName(module.input_gateway),
                    std::to_string(findGatewayInstanceId(module.input_gateway)),
                    std::to_string(0) /* 0-index is dedicated to gateway */
                ),
                To(
                    moduleName,
                    std::to_string(instanceId),
                    std::to_string(0)));  /* 0-index is dedicated to gateway */

            links.add(l);
        }

        if (module.output_gateway.val.dw != dsp_fw::ConnectorNodeId::kInvalidNodeId) {
            /* Connected to an output gateway */

            Link l(
                From(
                    moduleName,
                    std::to_string(instanceId),
                    std::to_string(0) /* 0-index is dedicated to gateway */
                ),
                To(
                    findGatewayTypeName(module.output_gateway),
                    std::to_string(findGatewayInstanceId(module.output_gateway)),
                    std::to_string(0)));  /* 0-index is dedicated to gateway */

            links.add(l);
        }
    }
    children.add(moduleCollection);

    /* Links between modules */
    for (auto &link : mTopology.links) {
        uint16_t fromModuleId, fromInstanceId;
        uint16_t toModuleId, toInstanceId;

        Topology::splitModuleInstanceId(link.mFromModuleInstanceId, fromModuleId, fromInstanceId);
        Topology::splitModuleInstanceId(link.mToModuleInstanceId, toModuleId, toInstanceId);

        auto fromName = findModuleEntryName(fromModuleId);
        auto toName = findModuleEntryName(toModuleId);

        Link l(
            From(
                fromName,
                std::to_string(fromInstanceId),
                std::to_string(link.mFromOutputId + 1) /* + 1 because 0 is dedicated to gateway */
            ),
            To(
                toName,
                std::to_string(toInstanceId),
                std::to_string(link.mToInputId + 1))); /* + 1 because 0 is dedicated to gateway */

        links.add(l);
    }

    return subsystem;
}

}
}
