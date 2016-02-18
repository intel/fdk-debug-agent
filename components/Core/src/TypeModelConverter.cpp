/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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
    for (auto &module : mSystem.getModuleEntries()) {
        addSubsystemSubType(typeMap, createModule(module.module_id));
    }

    /* gateways */
    for (auto &entry : dsp_fw::ConnectorNodeId::getTypeEnumHelper().getEnumToStringMap()) {
        addSubsystemSubType(typeMap, createGateway(entry.second));
    }

    /* Services */

    /* Currently the log service does not have endpoint*/
    addSubsystemServiceTypes(typeMap, logServiceTypeName, EndPoint::Direction::Outgoing,
                             logServiceEndPointCount);
    addSubsystemServiceTypes(typeMap, probeServiceTypeName, EndPoint::Direction::Bidirectional,
                             probeServiceEndPointCount);

    return std::make_shared<TypeModel>(createSystem(), typeMap);
}

std::shared_ptr<ifdk_objects::type::System> TypeModelConverter::createSystem()
{
    auto system = std::make_shared<ifdk_objects::type::System>();
    system->setName(systemName);
    system->getDescription().setValue(systemDescription);

    auto coll = std::make_shared<SubsystemRefCollection>(collectionName_subsystem);

    coll->add(SubsystemRef(subsystemName));
    system->getChildren().add(coll);

    return system;
}

std::shared_ptr<Type> TypeModelConverter::createSubsystem()
{
    auto subsystem = std::make_shared<Subsystem>();
    subsystem->setName(subsystemName);
    subsystem->getDescription().setValue(subsystemDescription);

    /* Characteristics */
    Characteristics &ch = subsystem->getCharacteristics();
    getSystemCharacteristics(ch);

    /* Children and categories */
    Children &children = subsystem->getChildren();
    Categories &categories = subsystem->getCategories();

    /* Static types */
    /* Asserting because these two collections are statically defined in the debug agent */
    assert(staticTypeCollections.size() == staticTypes.size());
    for (std::size_t i = 0; i < staticTypeCollections.size(); ++i) {
        auto coll = std::make_shared<TypeRefCollection>(staticTypeCollections[i]);

        coll->add(TypeRef(staticTypes[i]));
        children.add(coll);

        categories.add(std::make_shared<TypeRef>(staticTypes[i]));
    }

    /* Service */
    auto serviceColl = std::make_shared<ServiceRefCollection>(collectionName_service);

    for (auto &serviceName : staticServiceTypes) {
        serviceColl->add(ServiceRef(serviceName));

        categories.add(std::make_shared<ServiceRef>(serviceName));
    }
    children.add(serviceColl);

    /* Gateways */
    auto gatewayColl = std::make_shared<ComponentRefCollection>(collectionName_gateway);

    for (auto &gatewayPair : dsp_fw::ConnectorNodeId::getTypeEnumHelper().getEnumToStringMap()) {
        const std::string &gatewayName = gatewayPair.second;

        gatewayColl->add(ComponentRef(gatewayName));

        categories.add(std::make_shared<ComponentRef>(gatewayName));
    }
    children.add(gatewayColl);

    /* Modules*/
    auto compColl = std::make_shared<ComponentRefCollection>(collectionName_module);

    for (auto &module : mSystem.getModuleEntries()) {
        std::string moduleName = findModuleEntryName(module.module_id);

        compColl->add(ComponentRef(moduleName));

        categories.add(std::make_shared<ComponentRef>(moduleName));
    }
    children.add(compColl);

    return subsystem;
}

std::shared_ptr<Type> TypeModelConverter::createPipe()
{
    auto pipe = std::make_shared<Type>();

    pipe->setName(typeName_pipe);
    pipe->getDescription().setValue(typeDescription_pipe);

    /* Modules*/
    auto compColl = std::make_shared<ComponentRefCollection>(collectionName_module);

    for (auto &module : mSystem.getModuleEntries()) {
        std::string moduleName = findModuleEntryName(module.module_id);

        compColl->add(ComponentRef(moduleName));
    }
    pipe->getChildren().add(compColl);

    /* Task */
    auto coll = std::make_shared<TypeRefCollection>(collectionName_task);
    coll->add(TypeRef(typeName_task));
    pipe->getChildren().add(coll);

    return pipe;
}

std::shared_ptr<Type> TypeModelConverter::createTask()
{
    auto task = std::make_shared<Type>();

    task->setName(typeName_task);
    task->getDescription().setValue(typeDescription_task);

    /* Modules*/
    auto compColl = std::make_shared<ComponentRefCollection>(collectionName_module);

    for (auto &module : mSystem.getModuleEntries()) {
        std::string moduleName = findModuleEntryName(module.module_id);

        compColl->add(ComponentRef(moduleName));
    }
    task->getChildren().add(compColl);

    return task;
}

std::shared_ptr<Type> TypeModelConverter::createCore()
{
    auto core = std::make_shared<Type>();

    core->setName(typeName_core);
    core->getDescription().setValue(typeDescription_core);

    /* Task */
    auto coll = std::make_shared<TypeRefCollection>(collectionName_task);
    coll->add(TypeRef(typeName_task));
    core->getChildren().add(coll);

    return core;
}

std::shared_ptr<Type> TypeModelConverter::createGateway(const std::string &name)
{
    auto gateway = std::make_shared<Component>();

    gateway->setName(name);
    gateway->getDescription().setValue(subsystemDescription + " " + name);

    return gateway;
}

std::shared_ptr<Type> TypeModelConverter::createModule(uint16_t id)
{
    auto module = std::make_shared<Component>();
    auto name = findModuleEntryName(id);

    module->setName(name);
    module->getDescription().setValue(subsystemDescription + " " + name);

    /* Characteristics */
    Characteristics &ch = module->getCharacteristics();
    ch.add(Characteristic("ModuleId", std::to_string(id)));

    const dsp_fw::ModuleEntry &entry = findModuleEntry(id);
    util::Uuid uuid;
    uuid.fromOtherUuidType(entry.uuid);

    ch.add(Characteristic("UUID", uuid.toString()));

    return module;
}

std::shared_ptr<Type> TypeModelConverter::createService(const std::string &serviceTypeName,
                                                        std::size_t endPointCount)
{
    auto service = std::make_shared<Service>(serviceTypeName);
    service->getDescription().setValue(getServiceTypeDescription(serviceTypeName));

    // service children
    if (endPointCount > 0) {
        auto coll = std::make_shared<EndPointRefCollection>(collectionName_endpoint);
        coll->add(EndPointRef(getEndPointTypeName(serviceTypeName)));
        service->getChildren().add(coll);
    }

    return service;
}

std::shared_ptr<Type> TypeModelConverter::createEndPoint(const std::string &serviceTypeName,
                                                         EndPoint::Direction direction)
{
    std::string endPointTypeName = getEndPointTypeName(serviceTypeName);

    auto endpoint = std::make_shared<EndPoint>(endPointTypeName, direction);
    endpoint->getDescription().setValue(getEndPointTypeDescription(serviceTypeName));

    return endpoint;
}

void TypeModelConverter::addSubsystemServiceTypes(TypeModel::TypeMap &map,
                                                  const std::string &serviceTypeName,
                                                  EndPoint::Direction direction,
                                                  std::size_t endPointCount)
{
    addSubsystemSubType(map, createService(serviceTypeName, endPointCount));

    /* Adding endpoint type only if there is at least one endpoint */
    if (endPointCount > 0) {
        addSubsystemSubType(map, createEndPoint(serviceTypeName, direction));
    }
}

void TypeModelConverter::getSystemCharacteristics(Characteristics &ch)
{
    // Add FW config
    const dsp_fw::FwConfig &fwConfig = mSystem.getFwConfig();
    if (fwConfig.isFwVersionValid) {
        ch.add(
            Characteristic("Firmware version", std::to_string(fwConfig.fwVersion.major) + "." +
                                                   std::to_string(fwConfig.fwVersion.minor) + "." +
                                                   std::to_string(fwConfig.fwVersion.hotfix) + "." +
                                                   std::to_string(fwConfig.fwVersion.build)));
    }
    if (fwConfig.isMemoryReclaimedValid) {
        ch.add(Characteristic("Memory reclaimed", std::to_string(fwConfig.memoryReclaimed)));
    }
    if (fwConfig.isSlowClockFreqHzValid) {
        ch.add(
            Characteristic("Slow clock frequency (Hz)", std::to_string(fwConfig.slowClockFreqHz)));
    }
    if (fwConfig.isFastClockFreqHzValid) {
        ch.add(
            Characteristic("Fast clock frequency (Hz)", std::to_string(fwConfig.fastClockFreqHz)));
    }
    if (fwConfig.dmaBufferConfig.size() > 0) {
        size_t i = 0;
        for (auto dmaBufferConfig : fwConfig.dmaBufferConfig) {
            ch.add(Characteristic("DMA buffer config #" + std::to_string(i) + " min size (bytes)",
                                  std::to_string(dmaBufferConfig.min_size_bytes)));
            ch.add(Characteristic("DMA buffer config #" + std::to_string(i) + " max size (bytes)",
                                  std::to_string(dmaBufferConfig.max_size_bytes)));
            ++i;
        }
    }
    if (fwConfig.isAlhSupportLevelValid) {
        ch.add(Characteristic("Audio Hub Link support level",
                              std::to_string(fwConfig.alhSupportLevel)));
    }
    if (fwConfig.isIpcDlMailboxBytesValid) {
        ch.add(Characteristic("IPC down link (host to FW) mailbox size (bytes)",
                              std::to_string(fwConfig.ipcDlMailboxBytes)));
    }
    if (fwConfig.isIpcUlMailboxBytesValid) {
        ch.add(Characteristic("IPC up link (FW to host) mailbox size (bytes)",
                              std::to_string(fwConfig.ipcUlMailboxBytes)));
    }
    if (fwConfig.isTraceLogBytesValid) {
        ch.add(Characteristic("Size of trace log buffer per single core (bytes)",
                              std::to_string(fwConfig.traceLogBytes)));
    }
    if (fwConfig.isMaxPplCountValid) {
        ch.add(Characteristic("Maximum number of pipelines instances",
                              std::to_string(fwConfig.maxPplCount)));
    }
    if (fwConfig.isMaxAstateCountValid) {
        ch.add(Characteristic("Maximum number of A-state table entries",
                              std::to_string(fwConfig.maxAstateCount)));
    }
    if (fwConfig.isMaxModulePinCountValid) {
        ch.add(Characteristic("Maximum number of input or output pins per module",
                              std::to_string(fwConfig.maxModulePinCount)));
    }
    if (fwConfig.isModulesCountValid) {
        ch.add(Characteristic("Current total number of module entries loaded into the DSP",
                              std::to_string(fwConfig.modulesCount)));
    }
    if (fwConfig.isMaxModInstCountValid) {
        ch.add(Characteristic("Maximum module instance count",
                              std::to_string(fwConfig.maxModInstCount)));
    }
    if (fwConfig.isMaxLlTasksPerPriCountValid) {
        ch.add(Characteristic("Maximum number of LL tasks per priority",
                              std::to_string(fwConfig.maxLlTasksPerPriCount)));
    }
    if (fwConfig.isLlPriCountValid) {
        ch.add(Characteristic("Number of LL priorities", std::to_string(fwConfig.llPriCount)));
    }
    if (fwConfig.isMaxDpTasksCountValid) {
        ch.add(Characteristic("Maximum number of DP tasks per core",
                              std::to_string(fwConfig.maxDpTasksCount)));
    }

    // Add HW config
    const dsp_fw::HwConfig &hwConfig = mSystem.getHwConfig();
    if (hwConfig.isCavsVersionValid) {
        ch.add(Characteristic("cAVS Version", std::to_string(hwConfig.cavsVersion)));
    }
    if (hwConfig.isDspCoreCountValid) {
        ch.add(Characteristic("Number of cores", std::to_string(hwConfig.dspCoreCount)));
    }
    if (hwConfig.isMemPageSizeValid) {
        ch.add(Characteristic("Memory page size (bytes)", std::to_string(hwConfig.memPageSize)));
    }
    if (hwConfig.isTotalPhysicalMemoryPageValid) {
        ch.add(Characteristic("Total number of physical pages",
                              std::to_string(hwConfig.totalPhysicalMemoryPage)));
    }
    if (hwConfig.isI2sCapsValid) {
        ch.add(Characteristic("I2S version", std::to_string(hwConfig.i2sCaps.version)));
        size_t i = 0;
        for (auto controllerBaseAddr : hwConfig.i2sCaps.controllerBaseAddr) {

            ch.add(Characteristic("I2S controller #" + std::to_string(i++) + " base address",
                                  std::to_string(controllerBaseAddr)));
        }
    }
    if (hwConfig.isGpdmaCapsValid) {

        ch.add(Characteristic("LP GPDMA count" +
                              std::to_string(hwConfig.gpdmaCaps.lp_gateways.size())));
        ch.add(Characteristic("HP GPDMA count" +
                              std::to_string(hwConfig.gpdmaCaps.hp_gateways.size())));
    }
    if (hwConfig.isGatewayCountValid) {
        ch.add(
            Characteristic("Total number of DMA gateways", std::to_string(hwConfig.gatewayCount)));
    }
    if (hwConfig.isEbbCountValid) {
        ch.add(Characteristic("Number of SRAM memory banks", std::to_string(hwConfig.ebbCount)));
    }
}
}
}
