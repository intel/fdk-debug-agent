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

#pragma once

#include "cAVS/FirmwareTypes.hpp"
#include "cAVS/DynamicSizedFirmwareTypes.hpp"
#include "cAVS/FwConfig.hpp"
#include "cAVS/HwConfig.hpp"
#include "cAVS/Topology.hpp"
#include "IfdkObjects/Xml/TypeSerializer.hpp"
#include "IfdkObjects/Xml/InstanceSerializer.hpp"
#include "Util/StringHelper.hpp"


namespace debug_agent
{
namespace core
{

/** This class converts cAVS data model to generic data model */
class ModelConverter final
{
public:
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    /** Create a system type */
    static void getSystemType(ifdk_objects::type::System &system);

    /** Create a system intance */
    static void getSystemInstance(ifdk_objects::instance::System &system);

    /** Create a subsystem type */
    static void getSubsystemType(ifdk_objects::type::Subsystem &subsystem,
        const cavs::FwConfig &fwConfig, const cavs::HwConfig &hwConfig,
        const std::vector<cavs::ModuleEntry> &entries);

    /** Create a subsystem instance */
    static void getSubsystemInstance(ifdk_objects::instance::Subsystem &subsystem,
        const cavs::FwConfig &fwConfig, const cavs::HwConfig &hwConfig,
        const std::vector<cavs::ModuleEntry> &entries,
        const cavs::Topology &topology);

    /** Create system characteristics */
    static void getSystemCharacteristics(ifdk_objects::type::Characteristics &characteristics,
        const cavs::FwConfig &fwConfig, const cavs::HwConfig &hwConfig);

private:
    ModelConverter();

    /** Find a module entry using its id */
    static const cavs::ModuleEntry &findModuleEntry(uint32_t moduleId,
        const std::vector<cavs::ModuleEntry> &entries);

    /** Find a module entry name using its id */
    static std::string findModuleEntryName(uint32_t moduleId,
        const std::vector<cavs::ModuleEntry> &entries);

    /** Find a gateway type name using a connector id */
    static std::string findGatewayTypeName(const cavs::dsp_fw::ConnectorNodeId &connectorId);

    /** Find a gateway instance id using a connector id */
    static uint32_t findGatewayInstanceId(const cavs::dsp_fw::ConnectorNodeId &connectorId);
};

}
}


