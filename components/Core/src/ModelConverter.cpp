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

static const std::string systemName = "bxtn";
static const std::string systemDescription = "Broxton platform";
static const std::string systemId = "0";

static const std::string subsystemName = "cavs";
static const std::string subsystemDescription = "cAVS subsystem";
static const std::string subsystemId = "0";

void ModelConverter::getSystemType(type::System &system)
{
    system.setName(systemName);
    system.getDescription().setValue(systemDescription);

    auto coll = new type::SubsystemRefCollection("subsystems");
    coll->add(type::SubsystemRef(subsystemName));
    system.getChildren().add(coll);
}

void ModelConverter::getSystemInstance(instance::System &system)
{
    system.setTypeName(systemName);
    system.setInstanceId(systemId);

    auto coll = new instance::SubsystemRefCollection("subsystems");
    coll->add(instance::SubsystemRef(subsystemName, subsystemId));
    system.getChildren().add(coll);

}

void ModelConverter::getSubsystemType(type::Subsystem &subsystem,
    const FwConfig &fwConfig, const HwConfig &hwConfig,
    const std::vector<ModuleEntry> &entries)
{
    static const std::vector<std::string> staticTypeCollections = {
        "pipes", "cores", "tasks"
    };

    static const std::vector<std::string> staticTypes = {
        "pipe", "core", "task"
    };

    static const std::vector<std::string> staticServiceTypes = {
        "fwlogs"
    };

    static const std::vector<std::string> gateways = {
        "hda-host-out-gateway",
        "hda-host-in-gateway",
        "hda-link-out-gateway",
        "hda-link-in-gateway",
        "dmic-link-in-gateway"
    };

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
    auto serviceColl = new type::ServiceRefCollection("services");
    for (auto &serviceName : staticServiceTypes) {
        serviceColl->add(type::ServiceRef(serviceName));

        categories.add(new type::ServiceRef(serviceName));
    }
    children.add(serviceColl);

    /* Gateways */
    auto gatewayColl = new type::ComponentRefCollection("gateways");
    for (auto &gatewayName : gateways) {
        gatewayColl->add(type::ComponentRef(gatewayName));

        categories.add(new type::ComponentRef(gatewayName));
    }
    children.add(gatewayColl);

    /* Modules*/
    auto compColl = new type::ComponentRefCollection("modules");
    for (auto &module : entries) {
        std::string moduleName =
            util::StringHelper::getStringFromFixedSizeArray(module.name, sizeof(module.name));
        compColl->add(type::ComponentRef(moduleName));

        categories.add(new type::ComponentRef(moduleName));
    }
    children.add(compColl);
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
