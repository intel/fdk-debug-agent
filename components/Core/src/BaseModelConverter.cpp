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
#include "Core/BaseModelConverter.hpp"
#include "Util/StringHelper.hpp"

using namespace debug_agent::cavs;

namespace debug_agent
{
namespace core
{

/* definitions */
const std::string BaseModelConverter::systemName = "bxtn";
const std::string BaseModelConverter::systemDescription = "Broxton platform";
const std::string BaseModelConverter::systemId = "0";

const std::string BaseModelConverter::subsystemName = "cavs";
const std::string BaseModelConverter::subsystemDescription = "cAVS subsystem";
const std::string BaseModelConverter::subsystemId = "0";

const std::string BaseModelConverter::collectionName_pipe = "pipes";
const std::string BaseModelConverter::collectionName_core = "cores";
const std::string BaseModelConverter::collectionName_task = "tasks";
const std::string BaseModelConverter::collectionName_subsystem = "subsystems";
const std::string BaseModelConverter::collectionName_service = "services";
const std::string BaseModelConverter::collectionName_gateway = "gateways";
const std::string BaseModelConverter::collectionName_module = "modules";

const std::vector<std::string> BaseModelConverter::staticTypeCollections = {
    collectionName_pipe, collectionName_core, collectionName_task
};

const std::string BaseModelConverter::typeName_pipe = "pipe";
const std::string BaseModelConverter::typeName_core = "core";
const std::string BaseModelConverter::typeName_task = "task";
const std::string BaseModelConverter::typeDescription_pipe = "cAVS pipe type";
const std::string BaseModelConverter::typeDescription_core = "cAVS core type";
const std::string BaseModelConverter::typeDescription_task = "cAVS task type";

const std::vector<std::string> BaseModelConverter::staticTypes = {
    typeName_pipe, typeName_core, typeName_task
};

const std::string BaseModelConverter::logServiceTypeName = "fwlogs";

const std::vector<std::string> BaseModelConverter::staticServiceTypes = {
    logServiceTypeName
};

const std::map<dsp_fw::ConnectorNodeId::Type, std::string>
    BaseModelConverter::gatewayNames = {
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

const cavs::ModuleEntry &BaseModelConverter::findModuleEntry(uint32_t moduleId)
{
    const std::vector<cavs::ModuleEntry> &entries = mSystem.getModuleEntries();
    if (moduleId > entries.size()) {
        throw Exception("Wrong module id: " + std::to_string(moduleId) + " max: " +
            std::to_string(entries.size() - 1));
    }
    return entries[moduleId];
}

std::string BaseModelConverter::findModuleEntryName(uint32_t moduleId)
{
    const cavs::ModuleEntry &entry = findModuleEntry(moduleId);

    /** According to the SwAS, module type name is "module-<module name>", for instance
     * "module-aec".
     */
    return "module-" +
        util::StringHelper::getStringFromFixedSizeArray(entry.name, sizeof(entry.name));
}

std::string BaseModelConverter::findGatewayTypeName(
    const cavs::dsp_fw::ConnectorNodeId &connectorId)
{
    /* Casting the type into the associated enum */
    auto connectorType = static_cast<dsp_fw::ConnectorNodeId::Type>(connectorId.val.f.dma_type);

    /* Finding the gateway name */
    auto it = gatewayNames.find(connectorType);
    if (it == gatewayNames.end()) {
        throw Exception(
            "Unknown gateway type: " + std::to_string(connectorId.val.f.dma_type));
    }

    return it->second;
}

uint32_t BaseModelConverter::findGatewayInstanceId(
    const cavs::dsp_fw::ConnectorNodeId &connectorId)
{
    return connectorId.val.f.v_index;
}

}
}
