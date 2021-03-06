/*
 * Copyright (c) 2015-2016, Intel Corporation
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
#include "Core/InstanceModelConverter.hpp"
#include "Util/StringHelper.hpp"

using namespace debug_agent::cavs;
using namespace debug_agent::ifdk_objects::instance;

namespace debug_agent
{
namespace core
{

/* Audio format description is not available  */
const std::string audioFormatUnknown = "Unknown";

std::shared_ptr<InstanceModel> InstanceModelConverter::createModel()
{
    try {
        mSystem.getTopology(mTopology);
    } catch (cavs::System::Exception &e) {
        throw Exception("Cannot get topology from fw: " + std::string(e.what()));
    }

    initializeIntermediateStructures();

    InstanceModel::CollectionMap collectionMap;

    /* Subsystem instance */
    collectionMap[subsystemName] = createSubsystem();

    /* Other instances*/
    addInstanceCollection(collectionMap, typeName_core, createCore());
    addInstanceCollection(collectionMap, typeName_pipe, createPipe());
    addInstanceCollection(collectionMap, typeName_task, createTask());

    /* Module instances*/
    for (const auto &module : mSystem.getModuleHandler().getModuleEntries()) {
        addInstanceCollection(collectionMap, findModuleEntryName(module.module_id),
                              createModule(module.module_id));
    }

    /* Gateways */
    for (auto &entry : dsp_fw::ConnectorNodeId::getTypeEnumHelper().getEnumToStringMap()) {
        addInstanceCollection(collectionMap, entry.second, createGateway(entry.first));
    }

    /* Services */
    addServiceInstanceCollection(collectionMap, logServiceTypeName, logServiceEndPointCount);
    addServiceInstanceCollection(collectionMap, probeServiceTypeName, probeServiceEndPointCount);
    addServiceInstanceCollection(collectionMap, perfServiceTypeName, 0);

    return std::make_shared<InstanceModel>(collectionMap);
}

std::shared_ptr<ifdk_objects::instance::System> InstanceModelConverter::createSystem()
{
    auto system = std::make_shared<ifdk_objects::instance::System>();

    system->setTypeName(systemName);
    system->setInstanceId(systemId);

    auto coll = std::make_shared<SubsystemRefCollection>(collectionName_subsystem);
    coll->add(SubsystemRef(subsystemName, subsystemId));
    system->getChildren().add(coll);

    return system;
}

std::shared_ptr<BaseCollection> InstanceModelConverter::createSubsystem()
{
    auto subsystem = std::make_shared<Subsystem>();
    subsystem->setTypeName(subsystemName);
    subsystem->setInstanceId(subsystemId);

    Links &links = subsystem->getLinks();

    /* Parents */
    subsystem->getParents().add(std::make_shared<SystemRef>(systemName, systemId));

    /* Children*/
    Children &children = subsystem->getChildren();

    /* Pipes */
    auto pipeCollection = std::make_shared<InstanceRefCollection>(collectionName_pipe);

    for (auto &pipeline : mTopology.pipelines) {
        pipeCollection->add(InstanceRef(typeName_pipe, std::to_string(pipeline.id.getValue())));
    }
    children.add(pipeCollection);

    /* Cores */
    std::shared_ptr<InstanceRefCollection> coreCollection =
        std::make_shared<InstanceRefCollection>(collectionName_core);

    for (uint32_t coreId = 0; coreId < mSystem.getModuleHandler().getHwConfig().dspCoreCount;
         coreId++) {
        coreCollection->add(InstanceRef(typeName_core, std::to_string(coreId)));
    }
    children.add(coreCollection);

    /* Tasks */
    auto taskCollection = std::make_shared<InstanceRefCollection>(collectionName_task);

    for (auto &schedulersInfo : mTopology.schedulers) {
        for (auto &scheduler : schedulersInfo.scheduler_info) {
            for (auto &task : scheduler.task_info) {
                taskCollection->add(InstanceRef(typeName_task, std::to_string(task.task_id)));
            }
        }
    }
    children.add(taskCollection);

    /* Gateways */
    auto gatewayCollection = std::make_shared<ComponentRefCollection>(collectionName_gateway);

    for (auto &gateway : mTopology.gateways) {
        dsp_fw::ConnectorNodeId connector(gateway.id);

        gatewayCollection->add(ComponentRef(findGatewayTypeName(connector),
                                            std::to_string(findGatewayInstanceId(connector))));
    }
    children.add(gatewayCollection);

    /* Modules and links to gateway*/
    auto moduleCollection = std::make_shared<ComponentRefCollection>(collectionName_module);
    for (auto &moduleEntry : mTopology.moduleInstances) {

        auto &module = moduleEntry.second;

        /* Finding the module type to get the type name */
        auto moduleName = findModuleEntryName(module.id.moduleId);

        moduleCollection->add(ComponentRef(moduleName, std::to_string(module.id.instanceId)));

        if (module.input_gateway.val.dw != dsp_fw::ConnectorNodeId::kInvalidNodeId) {
            /* Connected to an input gateway */

            Link l(From(findGatewayTypeName(module.input_gateway),
                        std::to_string(findGatewayInstanceId(module.input_gateway)),
                        std::to_string(0) /* 0-index is dedicated to gateway */
                        ),
                   To(moduleName, std::to_string(module.id.instanceId),
                      std::to_string(0))); /* 0-index is dedicated to gateway */

            links.add(l);
        }

        if (module.output_gateway.val.dw != dsp_fw::ConnectorNodeId::kInvalidNodeId) {
            /* Connected to an output gateway */

            Link l(From(moduleName, std::to_string(module.id.instanceId),
                        std::to_string(0) /* 0-index is dedicated to gateway */
                        ),
                   To(findGatewayTypeName(module.output_gateway),
                      std::to_string(findGatewayInstanceId(module.output_gateway)),
                      std::to_string(0))); /* 0-index is dedicated to gateway */

            links.add(l);
        }
    }
    children.add(moduleCollection);

    /* Services */
    auto serviceCollection = std::make_shared<ServiceRefCollection>(collectionName_service);

    serviceCollection->add(ServiceRef(logServiceTypeName, serviceId));
    serviceCollection->add(ServiceRef(probeServiceTypeName, serviceId));
    serviceCollection->add(ServiceRef(perfServiceTypeName, serviceId));
    children.add(serviceCollection);

    /* Links between modules */
    for (auto &link : mTopology.links) {
        uint16_t fromModuleId = link.mFromModuleInstanceId.moduleId;
        uint16_t fromInstanceId = link.mFromModuleInstanceId.instanceId;

        uint16_t toModuleId = link.mToModuleInstanceId.moduleId;
        uint16_t toInstanceId = link.mToModuleInstanceId.instanceId;

        auto fromName = findModuleEntryName(fromModuleId);
        auto toName = findModuleEntryName(toModuleId);

        Link l(From(fromName, std::to_string(fromInstanceId), std::to_string(link.mFromOutputId)),
               To(toName, std::to_string(toInstanceId), std::to_string(link.mToInputId)));

        links.add(l);
    }

    auto coll = std::make_shared<SubsystemCollection>();
    coll->add(subsystem);

    return coll;
}

std::shared_ptr<BaseCollection> InstanceModelConverter::createService(const std::string &typeName,
                                                                      std::size_t endPointCount)
{
    /* Service */
    auto service = std::make_shared<Service>(typeName, serviceId);

    /* Parents */
    service->getParents().add(std::make_shared<SubsystemRef>(subsystemName, subsystemId));

    /* Children */
    if (endPointCount > 0) {
        auto endPointCollection = std::make_shared<EndPointRefCollection>(collectionName_endpoint);
        for (std::size_t endPointIndex = 0; endPointIndex < endPointCount; ++endPointIndex) {
            endPointCollection->add(
                EndPointRef(getEndPointTypeName(typeName), std::to_string(endPointIndex)));
        }

        service->getChildren().add(endPointCollection);
    }

    auto coll = std::make_shared<ServiceCollection>();
    coll->add(service);
    return coll;
}

std::shared_ptr<BaseCollection> InstanceModelConverter::createEndPoint(
    const std::string &serviceTypeName, std::size_t endPointCount)
{
    std::string endPointTypeName = getEndPointTypeName(serviceTypeName);
    auto coll = std::make_shared<EndPointCollection>();
    for (std::size_t endPointIndex = 0; endPointIndex < endPointCount; ++endPointIndex) {

        /* End point */
        auto endpoint = std::make_shared<EndPoint>(endPointTypeName, std::to_string(endPointIndex));

        /* Parents */
        endpoint->getParents().add(std::make_shared<ServiceRef>(serviceTypeName, serviceId));

        coll->add(endpoint);
    }
    return coll;
}

std::shared_ptr<BaseCollection> InstanceModelConverter::createPipe()
{
    auto coll = std::make_shared<InstanceCollection>();

    for (auto &pplProps : mTopology.pipelines) {
        auto pipeline = std::make_shared<Instance>();
        pipeline->setTypeName(typeName_pipe);
        pipeline->setInstanceId(std::to_string(pplProps.id.getValue()));

        /* Parents */
        pipeline->getParents().add(std::make_shared<SubsystemRef>(subsystemName, subsystemId));

        /* Children */
        Children &children = pipeline->getChildren();

        /* Tasks */
        auto taskCollection = std::make_shared<InstanceRefCollection>(collectionName_task);

        for (auto taskId : pplProps.ll_tasks) {
            taskCollection->add(InstanceRef(typeName_task, std::to_string(taskId)));
        }
        for (auto taskId : pplProps.dp_tasks) {
            taskCollection->add(InstanceRef(typeName_task, std::to_string(taskId)));
        }
        children.add(taskCollection);

        /* Modules */
        children.add(createModuleRef(pplProps.module_instances));

        coll->add(pipeline);
    }

    return coll;
}

std::shared_ptr<BaseCollection> InstanceModelConverter::createTask()
{
    auto coll = std::make_shared<InstanceCollection>();

    for (auto &schedulersInfo : mTopology.schedulers) {
        for (auto &scheduler : schedulersInfo.scheduler_info) {
            for (auto &task : scheduler.task_info) {

                auto taskModel = std::make_shared<Instance>();
                taskModel->setTypeName(typeName_task);
                taskModel->setInstanceId(std::to_string(task.task_id));

                /* Parent : core */
                taskModel->getParents().add(std::make_shared<InstanceRef>(
                    typeName_core, std::to_string(scheduler.core_id)));

                /* Parent : pipe */
                auto it = mTaskParents.find(task.task_id);
                if (it == mTaskParents.end()) {
                    throw Exception("Task with id=" + std::to_string(task.task_id) + " not found.");
                }
                for (auto pipeId : it->second.pipeIds) {
                    taskModel->getParents().add(std::make_shared<InstanceRef>(
                        typeName_pipe, std::to_string(pipeId.getValue())));
                }

                /* Children */
                Children &children = taskModel->getChildren();

                /* Modules */
                children.add(createModuleRef(task.module_instance_id));

                coll->add(taskModel);
            }
        }
    }

    return coll;
}

std::shared_ptr<BaseCollection> InstanceModelConverter::createCore()
{
    auto coll = std::make_shared<InstanceCollection>();
    for (auto &schedulersInfo : mTopology.schedulers) {
        if (!mTopology.schedulers.empty()) {
            /* Gettting core Id */
            auto &scheduler = schedulersInfo.scheduler_info[0];
            uint32_t coreId = scheduler.core_id;

            /* Creating core instance */
            auto core = std::make_shared<Instance>();
            core->setTypeName(typeName_core);
            core->setInstanceId(std::to_string(coreId));

            /* Parent : subsystem */
            core->getParents().add(std::make_shared<SubsystemRef>(subsystemName, subsystemId));

            /* Children: tasks */
            auto taskCollection = std::make_shared<InstanceRefCollection>(collectionName_task);

            for (auto &sched : schedulersInfo.scheduler_info) {
                for (auto &task : sched.task_info) {
                    taskCollection->add(InstanceRef(typeName_task, std::to_string(task.task_id)));
                }
            }
            core->getChildren().add(taskCollection);

            coll->add(core);
        }
    }

    return coll;
}

std::shared_ptr<BaseCollection> InstanceModelConverter::createModule(uint16_t requestedModuleId)
{
    auto coll = std::make_shared<ComponentCollection>();
    for (auto &moduleInstanceEntry : mTopology.moduleInstances) {

        auto &module = moduleInstanceEntry.second;

        if (requestedModuleId == module.id.moduleId) {

            /* Creating module instance */
            auto moduleModel = std::make_shared<Component>();
            moduleModel->setTypeName(findModuleEntryName(module.id.moduleId));
            moduleModel->setInstanceId(std::to_string(module.id.instanceId));

            /* Parents */
            auto entry = mModuleParents.find(module.id);
            if (entry == mModuleParents.end()) {
                throw Exception("Module instance with type_id=" +
                                std::to_string(module.id.moduleId) + " instance_id=" +
                                std::to_string(module.id.instanceId) + " not found.");
            }

            ModuleParents &parents = entry->second;

            /* Subsystem */
            moduleModel->getParents().add(
                std::make_shared<SubsystemRef>(subsystemName, subsystemId));

            /* Parents: pipe*/
            for (auto pipeId : parents.pipeIds) {
                moduleModel->getParents().add(std::make_shared<InstanceRef>(
                    typeName_pipe, std::to_string(pipeId.getValue())));
            }

            /* Parents: tasks */
            for (auto taskId : parents.taskIds) {
                moduleModel->getParents().add(
                    std::make_shared<InstanceRef>(typeName_task, std::to_string(taskId)));
            }

            /* Input pins : connectors */
            std::size_t pinId = 0;
            for (auto &pin : module.input_pins.pin_info) {

                moduleModel->getInputs().add(Input(std::to_string(pinId), pin.format.toString()));
                ++pinId;
            }

            /* output pins : connectors */
            pinId = 0;
            for (auto &pin : module.output_pins.pin_info) {

                moduleModel->getOutputs().add(Output(std::to_string(pinId), pin.format.toString()));
                ++pinId;
            }

            coll->add(moduleModel);
        }
    }

    return coll;
}

std::shared_ptr<BaseCollection> InstanceModelConverter::createGateway(
    const dsp_fw::ConnectorNodeId::Type &gatewayType)
{
    auto coll = std::make_shared<ComponentCollection>();
    for (auto &gateway : mTopology.gateways) {
        dsp_fw::ConnectorNodeId connector(gateway.id);

        if (connector.val.f.dma_type == gatewayType) {

            /* Creating gateway model */
            auto gatewayModel = std::make_shared<Component>();
            gatewayModel->setTypeName(findGatewayTypeName(connector));
            gatewayModel->setInstanceId(std::to_string(findGatewayInstanceId(connector)));

            /* Parent : subsystem */
            gatewayModel->getParents().add(
                std::make_shared<SubsystemRef>(subsystemName, subsystemId));

            /* Inputs and outputs */
            auto it = mGatewayDirections.find(gateway.id);
            if (it != mGatewayDirections.end()) {
                switch (it->second) {
                case GatewayDirection::Input:
                    gatewayModel->getOutputs().add(Output("0", audioFormatUnknown));
                    break;

                case GatewayDirection::Output:
                    gatewayModel->getInputs().add(Input("0", audioFormatUnknown));
                    break;

                default:
                    abort(); // should not occur
                }
            }

            coll->add(gatewayModel);
        }
    }
    return coll;
}

std::shared_ptr<RefCollection> InstanceModelConverter::createModuleRef(
    const std::vector<dsp_fw::CompoundModuleId> &compoundIdList)
{
    auto coll = std::make_shared<ComponentRefCollection>(collectionName_module);

    for (auto &compoundId : compoundIdList) {
        coll->add(ComponentRef(findModuleEntryName(compoundId.moduleId),
                               std::to_string(compoundId.instanceId)));
    }
    return coll;
}

void InstanceModelConverter::initializeIntermediateStructures()
{
    mModuleParents.clear();
    mTaskParents.clear();
    mGatewayDirections.clear();

    /* Initializing module map entries */
    for (auto &entry : mTopology.moduleInstances) {
        mModuleParents[entry.first] = ModuleParents();
    }

    /* Initializing tasks map entries */
    for (auto &schedulersInfo : mTopology.schedulers) {
        for (auto &scheduler : schedulersInfo.scheduler_info) {
            for (auto &task : scheduler.task_info) {
                mTaskParents[task.task_id] = TaskParents();
            }
        }
    }

    /* Iterating on pipes to find pipes that are task parents or module pararents */
    for (auto &pplProps : mTopology.pipelines) {
        for (auto taskId : pplProps.ll_tasks) {
            auto it = mTaskParents.find(taskId);
            if (it != mTaskParents.end()) {
                it->second.pipeIds.insert(pplProps.id);
            }
        }
        for (auto taskId : pplProps.dp_tasks) {
            auto it = mTaskParents.find(taskId);
            if (it != mTaskParents.end()) {
                it->second.pipeIds.insert(pplProps.id);
            }
        }
        for (auto compoundModuleId : pplProps.module_instances) {
            auto it = mModuleParents.find(compoundModuleId);
            if (it != mModuleParents.end()) {
                it->second.pipeIds.insert(pplProps.id);
            }
        }
    }

    /* Iterating on tasks, to find tasks that are module parents */
    for (auto &schedulersInfo : mTopology.schedulers) {
        for (auto &scheduler : schedulersInfo.scheduler_info) {
            for (auto &task : scheduler.task_info) {
                for (auto compoundModuleId : task.module_instance_id) {
                    auto it = mModuleParents.find(compoundModuleId);
                    if (it != mModuleParents.end()) {
                        it->second.taskIds.insert(task.task_id);
                    }
                }
            }
        }
    }

    /* Iterating on module instances to find gateway info */
    for (auto &entry : mTopology.moduleInstances) {
        auto &module = entry.second;

        /* Input gateway*/
        uint32_t inputGatewayId = module.input_gateway.val.dw;
        if (inputGatewayId != dsp_fw::ConnectorNodeId::kInvalidNodeId) {
            /* Setting input direction to gateway */
            mGatewayDirections[inputGatewayId] = GatewayDirection::Input;
        }

        /* Output gateway*/
        uint32_t outputGatewayId = module.output_gateway.val.dw;
        if (outputGatewayId != dsp_fw::ConnectorNodeId::kInvalidNodeId) {
            /* Setting output direction to gateway*/
            mGatewayDirections[outputGatewayId] = GatewayDirection::Output;
        }
    }
}

void InstanceModelConverter::addInstanceCollection(
    InstanceModel::CollectionMap &map, const std::string &typeName,
    std::shared_ptr<ifdk_objects::instance::BaseCollection> collection)
{
    map[subsystemName + "." + typeName] = collection;
}

void InstanceModelConverter::addServiceInstanceCollection(InstanceModel::CollectionMap &map,
                                                          const std::string &serviceTypeName,
                                                          std::size_t endPointCount)
{
    addInstanceCollection(map, serviceTypeName, createService(serviceTypeName, endPointCount));

    if (endPointCount > 0) {
        addInstanceCollection(map, getEndPointTypeName(serviceTypeName),
                              createEndPoint(serviceTypeName, endPointCount));
    }
}
}
}
