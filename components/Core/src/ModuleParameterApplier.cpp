/*
 * Copyright (c) 2016, Intel Corporation
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

#include "XmlHelper.hpp"
#include "Core/ModuleParameterApplier.hpp"
#include "Core/BaseModelConverter.hpp"
#include "Util/StringHelper.hpp"
#include "Util/AssertAlways.hpp"
#include "Util/convert.hpp"
#include "Util/Uuid.hpp"
#include <sstream>

namespace debug_agent
{

using namespace cavs;
using namespace parameter_serializer;

namespace core
{

/** Name of the ParamID mapping key */
static const std::string paramId("ParamId");

ModuleParameterApplier::ModuleParameterApplier(
    cavs::System &system,
    util::Locker<parameter_serializer::ParameterSerializer> &parameterSerializer)
    : mSystem(system), mParameterSerializer(parameterSerializer)
{
    /* Building (fdk module name, cavs module name) map */
    for (auto &module : mSystem.getModuleHandler().getModuleEntries()) {

        std::string cavsModuleName = module.getName();

        std::string fdkModuleName = BaseModelConverter::subsystemName + "." +
                                    BaseModelConverter::modulePrefix + cavsModuleName;

        mFdkToCavsModuleNames[fdkModuleName] = cavsModuleName;
    }
}

std::set<std::string> ModuleParameterApplier::getSupportedTypes() const
{
    std::set<std::string> moduleTypes;
    for (auto &entry : mFdkToCavsModuleNames) {
        moduleTypes.insert(entry.first);
    }
    return moduleTypes;
}

std::string ModuleParameterApplier::getParameterStructure(const std::string &type,
                                                          ParameterKind parameterKind)
{
    auto info = getModuleInfo(type);
    const std::string &moduleTypeUuid = info.second;

    /* Getting parameter block names */
    const std::vector<std::string> children =
        getModuleParameterNames(moduleTypeUuid, parameterKind);

    /* Iterating over parameter blocks */
    std::string parameters;
    for (auto &blockName : children) {
        try {
            /* Concatenating parameter block structure xml produced by parameter serializer
            * @todo: use libstructure instead
            */
            parameters += mParameterSerializer->getStructureXml(
                BaseModelConverter::subsystemName, moduleTypeUuid,
                ParameterSerializer::ParameterKind::Control, blockName);
        } catch (ParameterSerializer::Exception &e) {
            throw Exception("Failed to get structure: " + std::string(e.what()));
        };
    }

    /* Producing final xml... */
    std::stringstream out;
    auto &&tag = getParameterKindTag(parameterKind);
    out << "<" << tag << ">\n" << parameters << "</" << tag << ">\n";

    return out.str();
}

void ModuleParameterApplier::setParameterValue(const std::string &type, ParameterKind parameterKind,
                                               const std::string &instanceIdStr,
                                               const std::string &parameterXML)
{
    auto info = getModuleInfo(type);
    const uint16_t &moduleTypeId = info.first;
    const std::string &moduleTypeUuid = info.second;
    uint16_t instanceId = parseModuleInstanceId(instanceIdStr);

    /* Getting parameter block names */
    const std::vector<std::string> children =
        getModuleParameterNames(moduleTypeUuid, parameterKind);

    try {
        /** @todo: use libstructure instead of XmlHelper */
        XmlHelper xml(parameterXML);
        for (auto &blockName : children) {

            /* Retrieving xml value subtree associated to the current parameter block */
            std::string block = xml.getSubTree(getParameterXPath(parameterKind, blockName));

            /* Encoding it using the parameter serializer */
            util::Buffer parameterPayload;
            try {
                parameterPayload = mParameterSerializer->xmlToBinary(
                    BaseModelConverter::subsystemName, moduleTypeUuid, translate(parameterKind),
                    blockName, block);
            } catch (ParameterSerializer::Exception &e) {
                throw Exception("Xml to binary conversion failed: " + std::string(e.what()));
            };

            /* Getting firmware parameter id that matches the parameter name */
            auto paramId = getParamIdFromName(moduleTypeUuid, parameterKind, blockName);

            /* Sending binary data to the fw */
            try {
                mSystem.getModuleHandler().setModuleParameter(moduleTypeId, instanceId, paramId,
                                                              parameterPayload);
            } catch (ModuleHandler::Exception &e) {
                throw Exception("Cannot set parameter: " + std::string(e.what()));
            }
        }
    } catch (XmlHelper::Exception &e) {
        throw Exception("Cannot set parameter value because of xml error " + std::string(e.what()));
    }
}

std::string ModuleParameterApplier::getParameterValue(const std::string &type,
                                                      ParameterKind parameterKind,
                                                      const std::string &instanceIdStr)
{
    auto info = getModuleInfo(type);
    const uint16_t &moduleTypeId = info.first;
    const std::string &moduleTypeUuid = info.second;
    uint16_t instanceId = parseModuleInstanceId(instanceIdStr);
    auto &&tag = getParameterKindTag(parameterKind);

    std::ostringstream out;
    out << "<" << tag << ">\n";
    switch (parameterKind) {
    case ParameterKind::Info:
        out << getInfoParameterValue(moduleTypeId, instanceId);
        break;
    case ParameterKind::Control:
        out << getControlParameterValue(moduleTypeId, moduleTypeUuid, instanceId);
        break;
    }

    out << "</" << tag << ">\n";
    return out.str();
}

std::string ModuleParameterApplier::getInfoParameterValue(uint16_t moduleTypeId,
                                                          uint16_t instanceId)
{
    auto props = mSystem.getModuleHandler().getModuleInstanceProps(moduleTypeId, instanceId);

    std::ostringstream info;
    info << "<ParameterBlock Name=\"Memory state\">\n"
         << "    <IntegerParameter Name=\"Stack bytes\">" << props.stack_bytes
         << "</IntegerParameter>\n"
         << "    <IntegerParameter Name=\"BSS total bytes\">" << props.bss_total_bytes
         << "</IntegerParameter>\n"
         << "    <IntegerParameter Name=\"BSS used bytes\">" << props.bss_used_bytes
         << "</IntegerParameter>\n"
         << "</ParameterBlock>\n";
    return info.str();
}

std::string ModuleParameterApplier::getControlParameterValue(uint16_t moduleTypeId,
                                                             const std::string &moduleTypeUuid,
                                                             uint16_t instanceId)
{
    /* Getting parameter block names */
    const std::vector<std::string> children =
        getModuleParameterNames(moduleTypeUuid, ParameterKind::Control);

    std::string parameters;
    for (auto &blockName : children) {

        /* Getting firmware parameter id that matches the parameter name */
        auto paramId = getParamIdFromName(moduleTypeUuid, ParameterKind::Control, blockName);

        /* Get parameter value from FW */
        util::Buffer parameterPayload;
        try {
            parameterPayload =
                mSystem.getModuleHandler().getModuleParameter(moduleTypeId, instanceId, paramId);
        } catch (ModuleHandler::Exception &e) {
            throw Exception("Cannot get parameter: " + std::string(e.what()));
        }

        /* Converting it to xml using the parameter serializer, and concatening the result. */
        try {
            parameters += mParameterSerializer->binaryToXml(
                BaseModelConverter::subsystemName, moduleTypeUuid,
                translate(ParameterKind::Control), blockName, parameterPayload);
        } catch (ParameterSerializer::Exception &e) {
            throw Exception("Binary to xml conversion failed: " + std::string(e.what()));
        }
    }

    return parameters;
}

ParameterSerializer::ParameterKind ModuleParameterApplier::translate(ParameterKind kind)
{
    switch (kind) {
    case ParameterKind::Control:
        return ParameterSerializer::ParameterKind::Control;
    case ParameterKind::Info:
        return ParameterSerializer::ParameterKind::Info;
    }
    ASSERT_ALWAYS(false);
}

std::string ModuleParameterApplier::getParameterKindTag(ParameterKind parameterKind)
{
    switch (parameterKind) {
    case ParameterKind::Control:
        return "control_parameters";
    case ParameterKind::Info:
        return "info_parameters";
    }
    ASSERT_ALWAYS(false);
}

std::vector<std::string> ModuleParameterApplier::getModuleParameterNames(
    const std::string &moduleTypeName, ParameterKind parameterKind) const
{
    std::map<uint32_t, std::string> children;
    try {
        children = mParameterSerializer->getChildren(BaseModelConverter::subsystemName,
                                                     moduleTypeName, translate(parameterKind));
    } catch (ParameterSerializer::ElementNotFound &) {
        return {};
    } catch (ParameterSerializer::Exception &e) {
        throw Exception("Cannot get parameter map : " + std::string(e.what()));
    }

    std::vector<std::string> names;
    for (auto &it : children) {
        names.push_back(it.second);
    }
    return names;
}

std::string ModuleParameterApplier::getParameterXPath(ParameterKind parameterKind,
                                                      const std::string &paramName)
{
    return "/" + getParameterKindTag(parameterKind) + "/ParameterBlock[@Name='" + paramName + "']";
}

uint16_t ModuleParameterApplier::parseModuleInstanceId(const std::string &instanceIdStr)
{
    uint16_t value;
    if (!convertTo(instanceIdStr, value)) {
        throw Exception("Wrong module instance id : '" + instanceIdStr + "'");
    }
    return value;
}

dsp_fw::ParameterId ModuleParameterApplier::getParamIdFromName(const std::string moduleTypeName,
                                                               ParameterKind parameterKind,
                                                               const std::string parameterName)
{
    std::string paramIdAsString;
    try {
        paramIdAsString =
            mParameterSerializer->getMapping(BaseModelConverter::subsystemName, moduleTypeName,
                                             translate(parameterKind), parameterName, paramId);
    } catch (ParameterSerializer::Exception &e) {
        throw Exception("Cannot retrieve mapping data: " + std::string(e.what()));
    }

    dsp_fw::ParameterId::RawType paramId;
    if (!convertTo(paramIdAsString, paramId)) {

        throw Exception("Invalid mapping data: " + paramIdAsString);
    }

    return dsp_fw::ParameterId{paramId};
}

std::string ModuleParameterApplier::fdkModuleTypeToCavs(const std::string &fdkModuleType) const
{
    auto it = mFdkToCavsModuleNames.find(fdkModuleType);
    if (it == mFdkToCavsModuleNames.end()) {
        throw Exception("Unknown module fdk name : " + fdkModuleType);
    }
    return it->second;
}

std::pair<uint16_t, std::string> ModuleParameterApplier::getModuleInfo(
    const std::string &fdkModuleTypeName) const
{
    try {
        auto &entry =
            mSystem.getModuleHandler().findModuleEntry(fdkModuleTypeToCavs(fdkModuleTypeName));
        util::Uuid uuid;
        uuid.fromOtherUuidType(entry.uuid);
        return std::pair<uint16_t, std::string>(entry.module_id, uuid.toString());
    } catch (System::Exception &e) {
        throw Exception(e.what());
    }
}
}
}
