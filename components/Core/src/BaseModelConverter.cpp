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
#include "Core/BaseModelConverter.hpp"
#include "Util/StringHelper.hpp"

using namespace debug_agent::cavs;

namespace debug_agent
{
namespace core
{

/* definitions */

/** @todo: remove these hardcoded system names
 *
 * System name is the platform name, but currently there is no way to retrieve it, so deducing
 * it from the current OS.
 */
#if defined(_WIN32)
const std::string BaseModelConverter::systemName = "spt";
const std::string BaseModelConverter::systemDescription = "Sunrisepoint platform";
#elif defined(__linux__)
const std::string BaseModelConverter::systemName = "bxtn";
const std::string BaseModelConverter::systemDescription = "Broxton platform";
#else
#error("Unsupported OS")
#endif

const std::string BaseModelConverter::systemId = "0";

const std::string BaseModelConverter::subsystemName = "cavs";
const std::string BaseModelConverter::subsystemDescription = "cAVS subsystem";
const std::string BaseModelConverter::subsystemId = "0";

const std::string BaseModelConverter::collectionName_pipe = "pipes";
const std::string BaseModelConverter::collectionName_core = "cores";
const std::string BaseModelConverter::collectionName_task = "tasks";
const std::string BaseModelConverter::collectionName_subsystem = "subsystems";
const std::string BaseModelConverter::collectionName_service = "services";
const std::string BaseModelConverter::collectionName_endpoint = "endpoints";
const std::string BaseModelConverter::collectionName_gateway = "gateways";
const std::string BaseModelConverter::collectionName_module = "modules";

const std::vector<std::string> BaseModelConverter::staticTypeCollections = {
    collectionName_pipe, collectionName_core, collectionName_task};

const std::string BaseModelConverter::typeName_pipe = "pipe";
const std::string BaseModelConverter::typeName_core = "core";
const std::string BaseModelConverter::typeName_task = "task";
const std::string BaseModelConverter::typeDescription_pipe = "cAVS pipe type";
const std::string BaseModelConverter::typeDescription_core = "cAVS core type";
const std::string BaseModelConverter::typeDescription_task = "cAVS task type";

const std::vector<std::string> BaseModelConverter::staticTypes = {typeName_pipe, typeName_core,
                                                                  typeName_task};

const std::string BaseModelConverter::serviceId = "0";

const std::string BaseModelConverter::logServiceTypeName = "fwlogs";
const std::size_t BaseModelConverter::logServiceEndPointCount = 0;

const std::string BaseModelConverter::probeServiceTypeName = "probe";
const std::size_t BaseModelConverter::probeServiceEndPointCount = 8;

const std::string BaseModelConverter::perfServiceTypeName = "perf_measurement";

const std::string BaseModelConverter::modulePrefix("module-");

const std::vector<std::string> BaseModelConverter::staticServiceTypes = {logServiceTypeName};

const dsp_fw::ModuleEntry &BaseModelConverter::findModuleEntry(uint16_t moduleId)
{
    try {
        return mSystem.getModuleHandler().findModuleEntry(moduleId);
    } catch (System::Exception &e) {
        throw Exception("BaseModelConverter: can not find module: " + std::string(e.what()));
    }
}

std::string BaseModelConverter::findModuleEntryName(uint16_t moduleId)
{
    const dsp_fw::ModuleEntry &entry = findModuleEntry(moduleId);

    /** According to the SwAS, module type name is "module-<module name>", for instance
     * "module-aec".
     */
    return modulePrefix + entry.getName();
}

std::string BaseModelConverter::findGatewayTypeName(const dsp_fw::ConnectorNodeId &connectorId)
{
    /* Casting the type into the associated enum */
    auto connectorType = static_cast<dsp_fw::ConnectorNodeId::Type>(connectorId.val.f.dma_type);

    /* Finding the gateway name */
    auto &helper = dsp_fw::ConnectorNodeId::getTypeEnumHelper();
    if (!helper.isValid(connectorType)) {
        throw Exception("Unknown gateway type: " + std::to_string(connectorId.val.f.dma_type));
    }

    return helper.toString(connectorType);
}

uint32_t BaseModelConverter::findGatewayInstanceId(const dsp_fw::ConnectorNodeId &connectorId)
{
    return connectorId.val.f.v_index;
}
}
}
