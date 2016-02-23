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
#include <ParameterSerializer/ParameterSerializer.hpp>
#include <ParameterMgrPlatformConnector.h>
#include <ElementHandle.h>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <iostream>

using namespace debug_agent::util;

namespace debug_agent
{
namespace parameter_serializer
{

ParameterSerializer::ParameterSerializer(const std::string configurationFilePath,
                                         bool validationRequested)
    : mParameterMgrPlatformConnector(nullptr)
{
    auto parameterMgrPlatformConnector =
        std::make_unique<CParameterMgrPlatformConnector>(configurationFilePath);

    std::string error;
    if (validationRequested) {
        if (!parameterMgrPlatformConnector->setValidateSchemasOnStart(true, error)) {

            /** @todo Use log instead */
            std::cout << "[Error] Failed to request schemas validation: " << error;

            return;
        }
    }

    // Prevent the Parameter Framework from spawning its remote interface
    parameterMgrPlatformConnector->setForceNoRemoteInterface(true);

    if (!parameterMgrPlatformConnector->start(error)) {

        /** @todo Use log instead */
        std::cout << "[Error] Parameter framework fails to start: " << error;
    } else {

        mParameterMgrPlatformConnector = std::move(parameterMgrPlatformConnector);
    }
}

ParameterSerializer::~ParameterSerializer()
{
}

std::unique_ptr<ElementHandle> ParameterSerializer::getElement(const std::string &subsystemName,
                                                               const std::string &moduleName,
                                                               ParameterKind parameterKind) const
{
    checkParameterMgrPlatformConnector();

    std::string error;

    std::string rootElementName =
        mParameterMgrPlatformConnector->createElementHandle("/", error)->getName();
    if (rootElementName == "") {
        throw Exception("No root element name found");
    }

    // compute module control element path from URL
    std::string moduleControlPath = std::string("/") + rootElementName + "/" + subsystemName +
                                    "/categories/" + moduleName + "/" +
                                    parameterKindHelper().toString(parameterKind) + "/";

    std::unique_ptr<ElementHandle> moduleElementHandle(
        mParameterMgrPlatformConnector->createElementHandle(moduleControlPath, error));

    if (moduleElementHandle == nullptr) {
        throw Exception("Invalid parameters format: node for path \"" + moduleControlPath +
                        "\" not found");
    }

    return moduleElementHandle;
}

std::unique_ptr<ElementHandle> ParameterSerializer::getChildElementHandle(
    const std::string &subsystemName, const std::string &elementName, ParameterKind parameterKind,
    const std::string &parameterName) const
{
    checkParameterMgrPlatformConnector();

    std::unique_ptr<ElementHandle> elementHandle =
        getElement(subsystemName, elementName, parameterKind);

    std::string error;
    std::unique_ptr<ElementHandle> childElementHandle(
        mParameterMgrPlatformConnector->createElementHandle(
            elementHandle->getPath() + "/" + parameterName, error));

    if (childElementHandle == nullptr) {
        throw Exception("Child " + parameterName + " not found for " + elementHandle->getPath());
    }

    return childElementHandle;
}

void ParameterSerializer::stripFirstLine(std::string &document)
{
    std::size_t endLinePos = document.find("\n");
    assert(endLinePos != std::string::npos);
    document.erase(0, endLinePos + 1);
}

std::map<uint32_t, std::string> ParameterSerializer::getChildren(const std::string &subsystemName,
                                                                 const std::string &elementName,
                                                                 ParameterKind parameterKind) const
{
    checkParameterMgrPlatformConnector();

    std::unique_ptr<ElementHandle> elementHandle =
        getElement(subsystemName, elementName, parameterKind);

    std::map<uint32_t, std::string> children;
    uint32_t childId = 0;
    for (const auto &handle : elementHandle->getChildren()) {
        children[childId] = handle.getName();
        childId++;
    }
    return children;
}

std::string ParameterSerializer::getMapping(const std::string &subsystemName,
                                            const std::string &elementName,
                                            ParameterKind parameterKind,
                                            const std::string &parameterName,
                                            const std::string &key) const
{
    checkParameterMgrPlatformConnector();

    std::string paramId;

    std::unique_ptr<ElementHandle> elementHandle =
        getChildElementHandle(subsystemName, elementName, parameterKind, parameterName);

    if (!elementHandle->getMappingData(key, paramId)) {
        throw Exception("Mapping \"" + key + "\" not found for " + elementHandle->getPath());
    }

    return paramId;
}

util::Buffer ParameterSerializer::xmlToBinary(const std::string &subsystemName,
                                              const std::string &elementName,
                                              ParameterKind parameterKind,
                                              const std::string &parameterName,
                                              const std::string parameterAsXml) const
{
    checkParameterMgrPlatformConnector();

    std::unique_ptr<ElementHandle> childElementHandle =
        getChildElementHandle(subsystemName, elementName, parameterKind, parameterName);

    // Send XML string to PFW
    std::string error;
    if (!childElementHandle->setAsXML(parameterAsXml, error)) {
        throw Exception("Not able to set XML stream for " + childElementHandle->getPath() + " : " +
                        error);
    }

    // Read binary back from PFW
    util::Buffer buffer;
    if (!childElementHandle->getAsBytes(buffer, error)) {
        throw Exception("Not able to get element as bytes for " + childElementHandle->getPath() +
                        " : " + error);
    }
    return buffer;
}

std::string ParameterSerializer::binaryToXml(const std::string &subsystemName,
                                             const std::string &elementName,
                                             ParameterKind parameterKind,
                                             const std::string &parameterName,
                                             const util::Buffer &parameterPayload) const
{
    checkParameterMgrPlatformConnector();

    std::unique_ptr<ElementHandle> childElementHandle =
        getChildElementHandle(subsystemName, elementName, parameterKind, parameterName);

    // Send binary to PFW
    std::string error;
    if (!childElementHandle->setAsBytes(parameterPayload, error)) {
        throw Exception("Not able to set payload for " + childElementHandle->getName() + " : " +
                        error);
    }
    std::string result;
    if (!childElementHandle->getAsXML(result, error)) {
        throw Exception("Not able to get element as xml for " + childElementHandle->getPath() +
                        " : " + error);
    }
    // Remove first line which is XML document header
    stripFirstLine(result);
    return result;
}

std::string ParameterSerializer::getStructureXml(const std::string &subsystemName,
                                                 const std::string &elementName,
                                                 ParameterKind parameterKind,
                                                 const std::string &parameterName) const
{
    checkParameterMgrPlatformConnector();

    std::unique_ptr<ElementHandle> childElementHandle =
        getChildElementHandle(subsystemName, elementName, parameterKind, parameterName);

    std::string error;
    std::string result;
    if (!childElementHandle->getStructureAsXML(result, error)) {
        throw Exception("Not able to get element as structure xml for " +
                        childElementHandle->getPath() + " : " + error);
    }

    // Remove first line which is XML document header
    stripFirstLine(result);
    return result;
}

void ParameterSerializer::checkParameterMgrPlatformConnector() const
{
    if (mParameterMgrPlatformConnector == nullptr) {
        throw Exception("Platform connector not available");
    }
}
}
}
