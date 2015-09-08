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
#include "Core/TypeModelConverter.hpp"
#include "Util/StringHelper.hpp"
#include "Util/Uuid.hpp"
#include <map>

using namespace debug_agent::cavs;
using namespace debug_agent::ifdk_objects::type;

namespace debug_agent
{
namespace core
{

void TypeModelConverter::addSubsystemSubType(TypeModel::TypeMap &map,
    std::shared_ptr<ifdk_objects::type::Type> type)
{
    map[subsystemName + "." + type->getName()] = type;
}

std::shared_ptr<TypeModel> TypeModelConverter::createModel()
{
    TypeModel::TypeMap typeMap;

    /* Subsystem type : key is the subsystem name*/
    typeMap[subsystemName] = createSubsystem();

    /* Subsystem subtypes : key is <subsystem name>.<type name> */
    addSubsystemSubType(typeMap, createPipe());
    addSubsystemSubType(typeMap, createTask());
    addSubsystemSubType(typeMap, createCore());

    /* modules */
    for (uint32_t moduleId = 0; moduleId < mSystem.getModuleEntries().size(); ++moduleId) {
        addSubsystemSubType(typeMap, createModule(moduleId));
    }

    /* gateways */
    for (auto &it : gatewayNames) {
        addSubsystemSubType(typeMap, createGateway(it.second));
    }

    /* Log service */
    addSubsystemSubType(typeMap, createLogService());

    return std::shared_ptr<TypeModel>(
        new TypeModel(createSystem(), typeMap));
}

std::shared_ptr<ifdk_objects::type::System> TypeModelConverter::createSystem()
{
    auto system = std::shared_ptr<ifdk_objects::type::System>(new ifdk_objects::type::System());
    system->setName(systemName);
    system->getDescription().setValue(systemDescription);

    auto coll = std::shared_ptr<SubsystemRefCollection>(
        new SubsystemRefCollection(collectionName_subsystem));

    coll->add(SubsystemRef(subsystemName));
    system->getChildren().add(coll);

    return system;
}

std::shared_ptr<Type> TypeModelConverter::createSubsystem()
{
    auto subsystem = std::shared_ptr<Subsystem>(new Subsystem());
    subsystem->setName(subsystemName);
    subsystem->getDescription().setValue(subsystemDescription);

    /* Characteristics */
    Characteristics &ch = subsystem->getCharacteristics();
    getSystemCharacteristics(ch);

    /* Children and categories */
    Children &children = subsystem->getChildren();
    Categories &categories = subsystem->getCategories();

    /* Static types */
    assert(staticTypeCollections.size() == staticTypes.size());
    for (std::size_t i = 0; i < staticTypeCollections.size(); ++i) {
        auto coll = std::shared_ptr<TypeRefCollection>(
            new TypeRefCollection(staticTypeCollections[i]));

        coll->add(TypeRef(staticTypes[i]));
        children.add(coll);

        categories.add(new TypeRef(staticTypes[i]));
    }

    /* Service */
    auto serviceColl = std::shared_ptr<ServiceRefCollection>(
        new ServiceRefCollection(collectionName_service));

    for (auto &serviceName : staticServiceTypes) {
        serviceColl->add(ServiceRef(serviceName));

        categories.add(new ServiceRef(serviceName));
    }
    children.add(serviceColl);

    /* Gateways */
    auto gatewayColl = std::shared_ptr<ComponentRefCollection>(
        new ComponentRefCollection(collectionName_gateway));

    for (auto &gatewayPair : gatewayNames) {
        const std::string &gatewayName = gatewayPair.second;

        gatewayColl->add(ComponentRef(gatewayName));

        categories.add(new ComponentRef(gatewayName));
    }
    children.add(gatewayColl);

    /* Modules*/
    auto compColl = std::shared_ptr<ComponentRefCollection>(
        new ComponentRefCollection(collectionName_module));

    for (uint32_t i = 0; i < mSystem.getModuleEntries().size(); ++i) {
        std::string moduleName = findModuleEntryName(i);

        compColl->add(ComponentRef(moduleName));

        categories.add(new ComponentRef(moduleName));
    }
    children.add(compColl);

    return subsystem;
}


std::shared_ptr<Type> TypeModelConverter::createPipe()
{
    auto pipe =
        std::shared_ptr<Type>(new Type());

    pipe->setName(typeName_pipe);
    pipe->getDescription().setValue(typeDescription_pipe);

    /* Modules*/
    auto compColl = std::shared_ptr<ComponentRefCollection>(
        new ComponentRefCollection(collectionName_module));

    for (uint32_t i = 0; i < mSystem.getModuleEntries().size(); ++i) {
        std::string moduleName = findModuleEntryName(i);

        compColl->add(ComponentRef(moduleName));
    }
    pipe->getChildren().add(compColl);

    /* Task */
    auto coll = std::shared_ptr<TypeRefCollection>(new TypeRefCollection(collectionName_task));
    coll->add(TypeRef(typeName_task));
    pipe->getChildren().add(coll);

    return pipe;
}

std::shared_ptr<Type> TypeModelConverter::createTask()
{
    auto task = std::shared_ptr<Type>(new Type());

    task->setName(typeName_task);
    task->getDescription().setValue(typeDescription_task);

    /* Modules*/
    auto compColl = std::shared_ptr<ComponentRefCollection>(
        new ComponentRefCollection(collectionName_module));

    for (uint32_t i = 0; i < mSystem.getModuleEntries().size(); ++i) {
        std::string moduleName = findModuleEntryName(i);

        compColl->add(ComponentRef(moduleName));
    }
    task->getChildren().add(compColl);

    return task;
}

std::shared_ptr<Type> TypeModelConverter::createCore()
{
    auto core = std::shared_ptr<Type>(new Type());

    core->setName(typeName_core);
    core->getDescription().setValue(typeDescription_core);

    /* Task */
    auto coll = std::shared_ptr<TypeRefCollection>(new TypeRefCollection(collectionName_task));
    coll->add(TypeRef(typeName_task));
    core->getChildren().add(coll);

    return core;
}

std::shared_ptr<Type> TypeModelConverter::createGateway(const std::string &name)
{
    auto gateway = std::shared_ptr<Component>(new Component());

    gateway->setName(name);
    gateway->getDescription().setValue(subsystemDescription + " " + name);

    return gateway;
}

std::shared_ptr<Type> TypeModelConverter::createModule(uint32_t id)
{
    assert(id < mSystem.getModuleEntries().size());

    auto module = std::shared_ptr<Component>(new Component());
    auto name = findModuleEntryName(id);

    module->setName(name);
    module->getDescription().setValue(subsystemDescription + " " + name);

    /* Characteristics */
    Characteristics &ch = module->getCharacteristics();
    ch.add(Characteristic("ModuleId", std::to_string(id)));

    const ModuleEntry &entry = mSystem.getModuleEntries()[id];
    util::Uuid uuid;
    uuid.fromOtherUuidType(entry.uuid);

    ch.add(Characteristic("UUID", uuid.toString()));

    return module;
}

std::shared_ptr<Type> TypeModelConverter::createLogService()
{
    auto service = std::shared_ptr<Service>(new Service(logServiceTypeName));
    service->getDescription().setValue(logServiceDescription);

    return service;
}

void TypeModelConverter::getSystemCharacteristics(Characteristics &ch)
{
    // Add FW config
    const FwConfig &fwConfig = mSystem.getFwConfig();
    if (fwConfig.isFwVersionValid) {
        ch.add(Characteristic(
            "Firmware version",
            std::to_string(fwConfig.fwVersion.major) + "." +
            std::to_string(fwConfig.fwVersion.minor) + "." +
            std::to_string(fwConfig.fwVersion.hotfix) + "." +
            std::to_string(fwConfig.fwVersion.build)));
    }
    if (fwConfig.isMemoryReclaimedValid) {
        ch.add(Characteristic(
            "Memory reclaimed",
            std::to_string(fwConfig.memoryReclaimed)));
    }
    if (fwConfig.isSlowClockFreqHzValid) {
        ch.add(Characteristic(
            "Slow clock frequency (Hz)",
            std::to_string(fwConfig.slowClockFreqHz)));
    }
    if (fwConfig.isFastClockFreqHzValid) {
        ch.add(Characteristic(
            "Fast clock frequency (Hz)",
            std::to_string(fwConfig.fastClockFreqHz)));
    }
    if (fwConfig.dmaBufferConfig.size() > 0) {
        size_t i = 0;
        for (auto dmaBufferConfig : fwConfig.dmaBufferConfig) {
            ch.add(Characteristic(
                "DMA buffer config #" + std::to_string(i) + " min size (bytes)",
                std::to_string(dmaBufferConfig.min_size_bytes)));
            ch.add(Characteristic(
                "DMA buffer config #" + std::to_string(i) + " max size (bytes)",
                std::to_string(dmaBufferConfig.max_size_bytes)));
            ++i;
        }
    }
    if (fwConfig.isAlhSupportLevelValid) {
        ch.add(Characteristic(
            "Audio Hub Link support level",
            std::to_string(fwConfig.alhSupportLevel)));
    }
    if (fwConfig.isIpcDlMailboxBytesValid) {
        ch.add(Characteristic(
            "IPC down link (host to FW) mailbox size (bytes)",
            std::to_string(fwConfig.ipcDlMailboxBytes)));
    }
    if (fwConfig.isIpcUlMailboxBytesValid) {
        ch.add(Characteristic(
            "IPC up link (FW to host) mailbox size (bytes)",
            std::to_string(fwConfig.ipcUlMailboxBytes)));
    }
    if (fwConfig.isTraceLogBytesValid) {
        ch.add(Characteristic(
            "Size of trace log buffer per single core (bytes)",
            std::to_string(fwConfig.traceLogBytes)));
    }
    if (fwConfig.isMaxPplCountValid) {
        ch.add(Characteristic(
            "Maximum number of pipelines instances",
            std::to_string(fwConfig.maxPplCount)));
    }
    if (fwConfig.isMaxAstateCountValid) {
        ch.add(Characteristic(
            "Maximum number of A-state table entries",
            std::to_string(fwConfig.maxAstateCount)));
    }
    if (fwConfig.isMaxModulePinCountValid) {
        ch.add(Characteristic(
            "Maximum number of input or output pins per module",
            std::to_string(fwConfig.maxModulePinCount)));
    }
    if (fwConfig.isModulesCountValid) {
        ch.add(Characteristic(
            "Current total number of module entries loaded into the DSP",
            std::to_string(fwConfig.modulesCount)));
    }
    if (fwConfig.isMaxModInstCountValid) {
        ch.add(Characteristic(
            "Maximum module instance count",
            std::to_string(fwConfig.maxModInstCount)));
    }
    if (fwConfig.isMaxLlTasksPerPriCountValid) {
        ch.add(Characteristic(
            "Maximum number of LL tasks per priority",
            std::to_string(fwConfig.maxLlTasksPerPriCount)));
    }
    if (fwConfig.isLlPriCountValid) {
        ch.add(Characteristic(
            "Number of LL priorities",
            std::to_string(fwConfig.llPriCount)));
    }
    if (fwConfig.isMaxDpTasksCountValid) {
        ch.add(Characteristic(
            "Maximum number of DP tasks per core",
            std::to_string(fwConfig.maxDpTasksCount)));
    }

    // Add HW config
    const HwConfig &hwConfig = mSystem.getHwConfig();
    if (hwConfig.isCavsVersionValid) {
        ch.add(Characteristic(
            "cAVS Version",
            std::to_string(hwConfig.cavsVersion)));
    }
    if (hwConfig.isDspCoreCountValid) {
        ch.add(Characteristic(
            "Number of cores",
            std::to_string(hwConfig.dspCoreCount)));
    }
    if (hwConfig.isMemPageSizeValid) {
        ch.add(Characteristic(
            "Memory page size (bytes)",
            std::to_string(hwConfig.memPageSize)));
    }
    if (hwConfig.isTotalPhysicalMemoryPageValid) {
        ch.add(Characteristic(
            "Total number of physical pages",
            std::to_string(hwConfig.totalPhysicalMemoryPage)));
    }
    if (hwConfig.isI2sCapsValid) {
        ch.add(Characteristic(
            "I2S version",
            std::to_string(hwConfig.i2sCaps.version)));
        size_t i = 0;
        for (auto controllerBaseAddr : hwConfig.i2sCaps.controllerBaseAddr) {

            ch.add(Characteristic(
                "I2S controller #" + std::to_string(i++) + " base address",
                std::to_string(controllerBaseAddr)));
        }
    }
    if (hwConfig.isGatewayCountValid) {
        ch.add(Characteristic(
            "Total number of DMA gateways",
            std::to_string(hwConfig.gatewayCount)));
    }
    if (hwConfig.isEbbCountValid) {
        ch.add(Characteristic(
            "Number of SRAM memory banks",
            std::to_string(hwConfig.ebbCount)));
    }
}

}
}
