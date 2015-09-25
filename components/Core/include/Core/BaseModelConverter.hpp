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

#include "cAVS/System.hpp"
#include <stdexcept>
#include <vector>
#include <map>

namespace debug_agent
{
namespace core
{

/** This is model converter base class.
 *
 * It contains helper methods and constants that will be used for both instance and type model
 */
class BaseModelConverter
{
public:
    /** Thrown when model conversion has failed */
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    BaseModelConverter(cavs::System &system) : mSystem(system) {}
    virtual ~BaseModelConverter() {}

    static const std::string subsystemName;

protected:
    /* Defining model names as constants */
    static const std::string systemName;
    static const std::string systemDescription;
    static const std::string systemId;

    static const std::string subsystemDescription;
    static const std::string subsystemId;

    static const std::string collectionName_pipe;
    static const std::string collectionName_core;
    static const std::string collectionName_task;
    static const std::string collectionName_subsystem;
    static const std::string collectionName_service;
    static const std::string collectionName_gateway;
    static const std::string collectionName_module;

    /* List of static type collection names, i.e type names that are not retrieved dynamically  */
    static const std::vector<std::string> staticTypeCollections;

    /* Known cavs type names */
    static const std::string typeName_pipe;
    static const std::string typeName_core;
    static const std::string typeName_task;
    static const std::string typeDescription_pipe;
    static const std::string typeDescription_core;
    static const std::string typeDescription_task;

    /* List of static type names, i.e type names that are not retrieved dynamically  */
    static const std::vector<std::string> staticTypes;

    /* cavs log service type */
    static const std::string logServiceTypeName;
    static const std::string logServiceId;
    static const std::string logServiceDescription;

    /* list of static services types */
    static const std::vector<std::string> staticServiceTypes;

    /** Find a module entry using its id */
    const cavs::ModuleEntry &findModuleEntry(uint32_t moduleId);

    /** Find a module entry name using its id */
    std::string findModuleEntryName(uint32_t moduleId);

    /** Find a gateway type name using a connector id */
    static std::string findGatewayTypeName(const cavs::dsp_fw::ConnectorNodeId &connectorId);

    /** Find a gateway instance id using a connector id */
    static uint32_t findGatewayInstanceId(const cavs::dsp_fw::ConnectorNodeId &connectorId);

    cavs::System &mSystem;

private:
    BaseModelConverter(const BaseModelConverter&) = delete;
    BaseModelConverter &operator=(const BaseModelConverter&) = delete;
};

}
}


