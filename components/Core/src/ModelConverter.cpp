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
#include "Core/ModelConverter.hpp"
#include "Util/StringHelper.hpp"

using namespace debug_agent::cavs;
using namespace debug_agent::ifdk_objects;

namespace debug_agent
{
namespace core
{

/* Static definitions */

static const std::string systemName = "bxtn";
static const std::string systemDescription = "Broxton platform";
static const std::string systemId = "0";

static const std::string subsystemName = "cavs";
static const std::string subsystemDescription = "cAVS subsystem";
static const std::string subsystemId = "0";

static const std::string collectionName_pipe = "pipes";
static const std::string collectionName_core = "cores";
static const std::string collectionName_task = "tasks";
static const std::string collectionName_subsystem = "subsystems";
static const std::string collectionName_service = "services";
static const std::string collectionName_gateway = "gateways";
static const std::string collectionName_module = "modules";

static const std::vector<std::string> staticTypeCollections = {
    collectionName_pipe, collectionName_core, collectionName_task
};

static const std::string typeName_pipe = "pipe";
static const std::string typeName_core = "core";
static const std::string typeName_task = "task";

static const std::vector<std::string> staticTypes = {
    typeName_pipe, typeName_core, typeName_task
};

static const std::string logServiceTypeName = "fwlogs";

static const std::vector<std::string> staticServiceTypes = {
    logServiceTypeName
};

static const std::map<dsp_fw::ConnectorNodeId::Type, std::string> gatewayNames = {
    { dsp_fw::ConnectorNodeId::kHdaHostOutputClass, "hda-host-out-gateway" },
    { dsp_fw::ConnectorNodeId::kHdaHostInputClass, "hda-host-in-gateway" },
    { dsp_fw::ConnectorNodeId::kHdaHostInoutClass, "hda-host-inout-gateway" },
    { dsp_fw::ConnectorNodeId::kHdaLinkOutputClass, "hda-link-out-gateway" },
    { dsp_fw::ConnectorNodeId::kHdaLinkInputClass, "hda-link-in-gateway" },
    { dsp_fw::ConnectorNodeId::kHdaLinkInoutClass, "hda-link-inout-gateway" },
    { dsp_fw::ConnectorNodeId::kDmicLinkInputClass, "dmic-link-in-gateway" },
    { dsp_fw::ConnectorNodeId::kI2sLinkOutputClass, "i2s-link-out-gateway" },
    { dsp_fw::ConnectorNodeId::kI2sLinkInputClass, "i2s-link-in-gateway" },
    { dsp_fw::ConnectorNodeId::kSlimbusLinkOutputClass, "slimbus-link-out-gateway" },
    { dsp_fw::ConnectorNodeId::kSlimbusLinkInputClass, "slimbus-link-in-gateway" },
    { dsp_fw::ConnectorNodeId::kALHLinkOutputClass, "alh-link-out-gateway" },
    { dsp_fw::ConnectorNodeId::kALHLinkInputClass, "alh-link-in-gateway" }
};

const cavs::ModuleEntry &ModelConverter::findModuleEntry(uint32_t moduleId,
    const std::vector<cavs::ModuleEntry> &entries)
{
    if (moduleId > entries.size()) {
        throw Exception("Wrong module id: " + std::to_string(moduleId) + " max: " +
            std::to_string(entries.size() - 1));
    }
    return entries[moduleId];
}

std::string ModelConverter::findModuleEntryName(uint32_t moduleId,
    const std::vector<cavs::ModuleEntry> &entries)
{
    const cavs::ModuleEntry &entry = findModuleEntry(moduleId, entries);
    return "module." +
        util::StringHelper::getStringFromFixedSizeArray(entry.name, sizeof(entry.name));
}

std::string ModelConverter::findGatewayTypeName(const cavs::dsp_fw::ConnectorNodeId &connectorId)
{
    /* Casting the type into the associated enum */
    auto connectorType = static_cast<dsp_fw::ConnectorNodeId::Type>(connectorId.val.f.dma_type);

    /* Finding the gateway name */
    auto it = gatewayNames.find(connectorType);
    if (it == gatewayNames.end()) {
        throw Exception("Unknown gateway type: " + std::to_string(connectorId.val.f.dma_type));
    }

    return it->second;
}

uint32_t ModelConverter::findGatewayInstanceId(const cavs::dsp_fw::ConnectorNodeId &connectorId)
{
    return connectorId.val.f.v_index;
}

void ModelConverter::getSystemType(type::System &system)
{
    system.setName(systemName);
    system.getDescription().setValue(systemDescription);

    auto coll = new type::SubsystemRefCollection(collectionName_subsystem);
    coll->add(type::SubsystemRef(subsystemName));
    system.getChildren().add(coll);
}

void ModelConverter::getSystemInstance(instance::System &system)
{
    system.setTypeName(systemName);
    system.setInstanceId(systemId);

    auto coll = new instance::SubsystemRefCollection(collectionName_subsystem);
    coll->add(instance::SubsystemRef(subsystemName, subsystemId));
    system.getChildren().add(coll);

}

void ModelConverter::getSubsystemType(type::Subsystem &subsystem,
    const FwConfig &fwConfig, const HwConfig &hwConfig,
    const std::vector<ModuleEntry> &entries)
{
    /* Creating meta model */
    subsystem.setName(subsystemName);
    subsystem.getDescription().setValue(subsystemDescription);

    /* Hardcoded characteristics (temporary) */
    type::Characteristics &ch = subsystem.getCharacteristics();
    getSystemCharacteristics(ch, fwConfig, hwConfig);

    /* Children and categories */
    type::Children &children = subsystem.getChildren();
    type::Categories &categories = subsystem.getCategories();

    /* Static types */
    assert(staticTypeCollections.size() == staticTypes.size());
    for (std::size_t i = 0; i < staticTypeCollections.size(); ++i) {
        auto coll = new type::TypeRefCollection(staticTypeCollections[i]);
        coll->add(type::TypeRef(staticTypes[i]));
        children.add(coll);

        categories.add(new type::TypeRef(staticTypes[i]));
    }

    /* Service */
    auto serviceColl = new type::ServiceRefCollection(collectionName_service);
    for (auto &serviceName : staticServiceTypes) {
        serviceColl->add(type::ServiceRef(serviceName));

        categories.add(new type::ServiceRef(serviceName));
    }
    children.add(serviceColl);

    /* Gateways */
    auto gatewayColl = new type::ComponentRefCollection(collectionName_gateway);
    for (auto &gatewayPair : gatewayNames) {
        const std::string &gatewayName = gatewayPair.second;

        gatewayColl->add(type::ComponentRef(gatewayName));

        categories.add(new type::ComponentRef(gatewayName));
    }
    children.add(gatewayColl);

    /* Modules*/
    auto compColl = new type::ComponentRefCollection(collectionName_module);
    for (auto &module : entries) {
        std::string moduleName =
            util::StringHelper::getStringFromFixedSizeArray(module.name, sizeof(module.name));
        compColl->add(type::ComponentRef(moduleName));

        categories.add(new type::ComponentRef(moduleName));
    }
    children.add(compColl);
}

void ModelConverter::getSubsystemInstance(ifdk_objects::instance::Subsystem &subsystem,
    const cavs::FwConfig &fwConfig, const cavs::HwConfig &hwConfig,
    const std::vector<cavs::ModuleEntry> &entries,
    const cavs::Topology &topology)
{
    subsystem.setTypeName(subsystemName);
    subsystem.setInstanceId(subsystemId);

    instance::Links &links = subsystem.getLinks();

    /* Parents */
    subsystem.getParents().add(new instance::SystemRef(systemName, systemId));

    /* Children*/
    instance::Children &children = subsystem.getChildren();

    /* Pipes */
    auto pipeCollection = new instance::InstanceRefCollection(collectionName_pipe);
    for (auto &pipeline : topology.pipelines) {
        pipeCollection->add(instance::InstanceRef(typeName_pipe, std::to_string(pipeline.id)));
    }
    children.add(pipeCollection);

    /* Cores */
    auto coreCollection = new instance::InstanceRefCollection(collectionName_core);
    if (!hwConfig.isDspCoreCountValid) {
        throw Exception("Core count is invalid.");
    }
    for (uint32_t coreId = 0; coreId < hwConfig.dspCoreCount; coreId++) {
        coreCollection->add(instance::InstanceRef(typeName_core, std::to_string(coreId)));
    }
    children.add(coreCollection);

    /* Tasks */
    auto taskCollection = new instance::InstanceRefCollection(collectionName_task);
    for (auto &schedulersInfo : topology.schedulers) {
        for (auto &scheduler : schedulersInfo.scheduler_info) {
            for (auto &task : scheduler.task_info) {
                taskCollection->add(
                    instance::InstanceRef(typeName_task, std::to_string(task.task_id)));
            }
        }
    }
    children.add(taskCollection);

    /* Gateways */
    auto gatewayCollection = new instance::ComponentRefCollection(collectionName_gateway);
    for (auto &gateway : topology.gateways) {
        dsp_fw::ConnectorNodeId connector(gateway.id);

        gatewayCollection->add(instance::ComponentRef(findGatewayTypeName(connector),
            std::to_string(findGatewayInstanceId(connector))));
    }
    children.add(gatewayCollection);

    /* Modules and links to gateway*/
    auto moduleCollection = new instance::ComponentRefCollection(collectionName_module);
    for (auto &moduleEntry : topology.moduleInstances) {

        auto &module = moduleEntry.second;
        uint16_t moduleId, instanceId;
        Topology::splitModuleInstanceId(module.id, moduleId, instanceId);

        /* Finding the module type to get the type name */
        auto moduleName = findModuleEntryName(moduleId, entries);

        moduleCollection->add(instance::ComponentRef(moduleName, std::to_string(instanceId)));

        if (module.input_gateway.val.dw != dsp_fw::ConnectorNodeId::kInvalidNodeId) {
            /* Connected to an input gateway */

            instance::Link l(
                instance::From(
                    findGatewayTypeName(module.input_gateway),
                    std::to_string(findGatewayInstanceId(module.input_gateway)),
                    std::to_string(0) /* 0-index is dedicated to gateway */
                ),
                instance::To(
                    moduleName,
                    std::to_string(instanceId),
                    std::to_string(0)));  /* 0-index is dedicated to gateway */

            links.add(l);
        }

        if (module.output_gateway.val.dw != dsp_fw::ConnectorNodeId::kInvalidNodeId) {
            /* Connected to an output gateway */

            instance::Link l(
                instance::From(
                    moduleName,
                    std::to_string(instanceId),
                    std::to_string(0) /* 0-index is dedicated to gateway */
                ),
                instance::To(
                    findGatewayTypeName(module.output_gateway),
                    std::to_string(findGatewayInstanceId(module.output_gateway)),
                    std::to_string(0)));  /* 0-index is dedicated to gateway */

            links.add(l);
        }
    }
    children.add(moduleCollection);

    /* Links between modules */
    for (auto &link : topology.links) {
        uint16_t fromModuleId, fromInstanceId;
        uint16_t toModuleId, toInstanceId;

        Topology::splitModuleInstanceId(link.mFromModuleInstanceId, fromModuleId, fromInstanceId);
        Topology::splitModuleInstanceId(link.mToModuleInstanceId, toModuleId, toInstanceId);

        auto fromName = findModuleEntryName(fromModuleId, entries);
        auto toName = findModuleEntryName(toModuleId, entries);

        instance::Link l(
            instance::From(
                fromName,
                std::to_string(fromInstanceId),
                std::to_string(link.mFromOutputId + 1) /* + 1 because 0 is dedicated to gateway */
            ),
            instance::To(
                toName,
                std::to_string(toInstanceId),
                std::to_string(link.mToInputId + 1))); /* + 1 because 0 is dedicated to gateway */

        links.add(l);
    }
}

void ModelConverter::getSystemCharacteristics(ifdk_objects::type::Characteristics &ch,
    const FwConfig &fwConfig, const HwConfig &hwConfig)
{
    // Add FW config
    if (fwConfig.isFwVersionValid) {
        ch.add(type::Characteristic(
            "Firmware version",
            std::to_string(fwConfig.fwVersion.major) + "." +
            std::to_string(fwConfig.fwVersion.minor) + "." +
            std::to_string(fwConfig.fwVersion.hotfix) + "." +
            std::to_string(fwConfig.fwVersion.build)));
    }
    if (fwConfig.isMemoryReclaimedValid) {
        ch.add(type::Characteristic(
            "Memory reclaimed",
            std::to_string(fwConfig.memoryReclaimed)));
    }
    if (fwConfig.isSlowClockFreqHzValid) {
        ch.add(type::Characteristic(
            "Slow clock frequency (Hz)",
            std::to_string(fwConfig.slowClockFreqHz)));
    }
    if (fwConfig.isFastClockFreqHzValid) {
        ch.add(type::Characteristic(
            "Fast clock frequency (Hz)",
            std::to_string(fwConfig.fastClockFreqHz)));
    }
    if (fwConfig.dmaBufferConfig.size() > 0) {
        size_t i = 0;
        for (auto dmaBufferConfig : fwConfig.dmaBufferConfig) {
            ch.add(type::Characteristic(
                "DMA buffer config #" + std::to_string(i) + " min size (bytes)",
                std::to_string(dmaBufferConfig.min_size_bytes)));
            ch.add(type::Characteristic(
                "DMA buffer config #" + std::to_string(i) + " max size (bytes)",
                std::to_string(dmaBufferConfig.max_size_bytes)));
            ++i;
        }
    }
    if (fwConfig.isAlhSupportLevelValid) {
        ch.add(type::Characteristic(
            "Audio Hub Link support level",
            std::to_string(fwConfig.alhSupportLevel)));
    }
    if (fwConfig.isIpcDlMailboxBytesValid) {
        ch.add(type::Characteristic(
            "IPC down link (host to FW) mailbox size (bytes)",
            std::to_string(fwConfig.ipcDlMailboxBytes)));
    }
    if (fwConfig.isIpcUlMailboxBytesValid) {
        ch.add(type::Characteristic(
            "IPC up link (FW to host) mailbox size (bytes)",
            std::to_string(fwConfig.ipcUlMailboxBytes)));
    }
    if (fwConfig.isTraceLogBytesValid) {
        ch.add(type::Characteristic(
            "Size of trace log buffer per single core (bytes)",
            std::to_string(fwConfig.traceLogBytes)));
    }
    if (fwConfig.isMaxPplCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of pipelines instances",
            std::to_string(fwConfig.maxPplCount)));
    }
    if (fwConfig.isMaxAstateCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of A-state table entries",
            std::to_string(fwConfig.maxAstateCount)));
    }
    if (fwConfig.isMaxModulePinCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of input or output pins per module",
            std::to_string(fwConfig.maxModulePinCount)));
    }
    if (fwConfig.isModulesCountValid) {
        ch.add(type::Characteristic(
            "Current total number of module entries loaded into the DSP",
            std::to_string(fwConfig.modulesCount)));
    }
    if (fwConfig.isMaxModInstCountValid) {
        ch.add(type::Characteristic(
            "Maximum module instance count",
            std::to_string(fwConfig.maxModInstCount)));
    }
    if (fwConfig.isMaxLlTasksPerPriCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of LL tasks per priority",
            std::to_string(fwConfig.maxLlTasksPerPriCount)));
    }
    if (fwConfig.isLlPriCountValid) {
        ch.add(type::Characteristic(
            "Number of LL priorities",
            std::to_string(fwConfig.llPriCount)));
    }
    if (fwConfig.isMaxDpTasksCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of DP tasks per core",
            std::to_string(fwConfig.maxDpTasksCount)));
    }

    // Add HW config
    if (hwConfig.isCavsVersionValid) {
        ch.add(type::Characteristic(
            "cAVS Version",
            std::to_string(hwConfig.cavsVersion)));
    }
    if (hwConfig.isDspCoreCountValid) {
        ch.add(type::Characteristic(
            "Number of cores",
            std::to_string(hwConfig.dspCoreCount)));
    }
    if (hwConfig.isMemPageSizeValid) {
        ch.add(type::Characteristic(
            "Memory page size (bytes)",
            std::to_string(hwConfig.memPageSize)));
    }
    if (hwConfig.isTotalPhysicalMemoryPageValid) {
        ch.add(type::Characteristic(
            "Total number of physical pages",
            std::to_string(hwConfig.totalPhysicalMemoryPage)));
    }
    if (hwConfig.isI2sCapsValid) {
        ch.add(type::Characteristic(
            "I2S version",
            std::to_string(hwConfig.i2sCaps.version)));
        size_t i = 0;
        for (auto controllerBaseAddr : hwConfig.i2sCaps.controllerBaseAddr) {

            ch.add(type::Characteristic(
                "I2S controller #" + std::to_string(i++) + " base address",
                std::to_string(controllerBaseAddr)));
        }
    }
    if (hwConfig.isGatewayCountValid) {
        ch.add(type::Characteristic(
            "Total number of DMA gateways",
            std::to_string(hwConfig.gatewayCount)));
    }
    if (hwConfig.isEbbCountValid) {
        ch.add(type::Characteristic(
            "Number of SRAM memory banks",
            std::to_string(hwConfig.ebbCount)));
    }
}

}
}
