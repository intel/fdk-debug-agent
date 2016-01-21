/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "XmlHelper.hpp"
#include "Core/ModuleParameterApplier.hpp"
#include "Core/BaseModelConverter.hpp"
#include "Util/StringHelper.hpp"
#include "Util/AssertAlways.hpp"
#include "Util/convert.hpp"
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
    for (auto &module : mSystem.getModuleEntries()) {

        std::string cavsModuleName =
            util::StringHelper::getStringFromFixedSizeArray(module.name, sizeof(module.name));

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
    std::string cavsModuleType = fdkModuleTypeToCavs(type);

    /* Getting parameter block names */
    const std::vector<std::string> children =
        getModuleParameterNames(cavsModuleType, parameterKind);

    /* Iterating over parameter blocks */
    std::string parameters;
    for (auto &blockName : children) {
        try {
            /* Concatenating parameter block structure xml produced by parameter serializer
            * @todo: use libstructure instead
            */
            parameters += mParameterSerializer->getStructureXml(
                BaseModelConverter::subsystemName, cavsModuleType,
                ParameterSerializer::ParameterKind::Control, blockName);
        } catch (ParameterSerializer::Exception &e) {
            throw Exception("Failed to get structure: " + std::string(e.what()));
        };
    }

    /* Producing final xml... */
    std::stringstream out;
    out << "<" << getParameterKindTag(parameterKind) << ">\n" << parameters << "</"
        << getParameterKindTag(parameterKind) << ">\n";

    return out.str();
}

void ModuleParameterApplier::setParameterValue(const std::string &type, ParameterKind parameterKind,
                                               const std::string &instanceIdStr,
                                               const std::string &parameterXML)
{
    std::string cavsModuleType = fdkModuleTypeToCavs(type);
    uint16_t moduleTypeId = getModuleTypeId(cavsModuleType);
    uint16_t instanceId = parseModuleInstanceId(instanceIdStr);

    /* Getting parameter block names */
    const std::vector<std::string> children =
        getModuleParameterNames(cavsModuleType, parameterKind);

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
                    BaseModelConverter::subsystemName, cavsModuleType, translate(parameterKind),
                    blockName, block);
            } catch (ParameterSerializer::Exception &e) {
                throw Exception("Xml to binary conversion failed: " + std::string(e.what()));
            };

            /* Getting firmware parameter id that matches the parameter name */
            auto paramId = getParamIdFromName(cavsModuleType, parameterKind, blockName);

            /* Sending binary data to the fw */
            try {
                mSystem.setModuleParameter(moduleTypeId, instanceId, paramId, parameterPayload);
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
    std::string cavsModuleType = fdkModuleTypeToCavs(type);
    uint16_t moduleTypeId = getModuleTypeId(cavsModuleType);
    uint16_t instanceId = parseModuleInstanceId(instanceIdStr);

    /* Getting parameter block names */
    const std::vector<std::string> children =
        getModuleParameterNames(cavsModuleType, parameterKind);

    std::string parameters;
    for (auto &blockName : children) {

        /* Getting firmware parameter id that matches the parameter name */
        auto paramId = getParamIdFromName(cavsModuleType, parameterKind, blockName);

        /* Get parameter value from FW */
        util::Buffer parameterPayload;
        try {
            mSystem.getModuleParameter(moduleTypeId, instanceId, paramId, parameterPayload);
        } catch (ModuleHandler::Exception &e) {
            throw Exception("Cannot get parameter: " + std::string(e.what()));
        }

        /* Converting it to xml using the parameter serializer, and concatening the result. */
        try {
            parameters += mParameterSerializer->binaryToXml(
                BaseModelConverter::subsystemName, cavsModuleType, translate(parameterKind),
                blockName, parameterPayload);
        } catch (ParameterSerializer::Exception &e) {
            throw Exception("Binary to xml conversion failed: " + std::string(e.what()));
        }
    }

    /** Producing the final xml rersult
    *
    * @todo use libstructure instead */
    std::stringstream out;
    out << "<" << getParameterKindTag(parameterKind) << ">\n" << parameters << "</"
        << getParameterKindTag(parameterKind) << ">\n";

    return out.str();
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

std::string ModuleParameterApplier::fdkModuleTypeToCavs(const std::string &fdkModuleType)
{
    auto it = mFdkToCavsModuleNames.find(fdkModuleType);
    if (it == mFdkToCavsModuleNames.end()) {
        throw Exception("Unknown module fdk name : " + fdkModuleType);
    }
    return it->second;
}

uint16_t ModuleParameterApplier::getModuleTypeId(const std::string &cavsModuleType)
{
    const std::vector<dsp_fw::ModuleEntry> &entries = mSystem.getModuleEntries();
    for (auto &entry : entries) {
        if (cavsModuleType ==
            util::StringHelper::getStringFromFixedSizeArray(entry.name, sizeof(entry.name))) {
            return entry.module_id;
        }
    }

    throw Exception("Unknown module type : '" + cavsModuleType + "'");
}
}
}
