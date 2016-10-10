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
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    BaseModelConverter(cavs::System &system) : mSystem(system) {}
    virtual ~BaseModelConverter() {}

    static const std::string subsystemName;
    static const std::string logServiceTypeName;
    static const std::size_t logServiceEndPointCount;
    static const std::string probeServiceTypeName;
    static const std::size_t probeServiceEndPointCount;
    static const std::string perfServiceTypeName;
    static const std::string modulePrefix;

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
    static const std::string collectionName_endpoint;
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

    /* Service instance identifier
     * Currently it can exist only one instance of a service type, so serviceId="0"
     */
    static const std::string serviceId;

    /* list of static services types */
    static const std::vector<std::string> staticServiceTypes;

    /** Find a module entry using its id */
    const cavs::dsp_fw::ModuleEntry &findModuleEntry(uint16_t moduleId);

    /** Find a module entry name using its id */
    std::string findModuleEntryName(uint16_t moduleId);

    /** Find a gateway type name using a connector id */
    static std::string findGatewayTypeName(const cavs::dsp_fw::ConnectorNodeId &connectorId);

    /** Find a gateway instance id using a connector id */
    static uint32_t findGatewayInstanceId(const cavs::dsp_fw::ConnectorNodeId &connectorId);

    /** Return endpoint type from service type */
    static std::string getEndPointTypeName(const std::string &serviceTypeName)
    {
        /* This convention is defined by the SwAS */
        return serviceTypeName + ".endpoint";
    }

    static std::string getServiceTypeDescription(const std::string &serviceTypeName)
    {
        /* Ex: cAVS subsystem probe service */
        return subsystemDescription + " " + serviceTypeName + " service";
    }

    static std::string getEndPointTypeDescription(const std::string &serviceTypeName)
    {
        /* Ex: cAVS subsystem probe endpoint */
        return subsystemDescription + " " + serviceTypeName + " endpoint";
    }

    cavs::System &mSystem;

private:
    BaseModelConverter(const BaseModelConverter &) = delete;
    BaseModelConverter &operator=(const BaseModelConverter &) = delete;
};
}
}
