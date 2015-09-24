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
#include <ParameterSerializer/ParameterSerializer.hpp>
#include <ParameterMgrPlatformConnector.h>
#include <ElementHandle.h>
#include <string>
#include <vector>
#include <map>
#include <cassert>

using namespace debug_agent::util;

namespace debug_agent
{
namespace parameterSerializer
{

ParameterSerializer::ParameterSerializer(const std::string configurationFilePath) :
mParameterMgrPlatformConnector(std::make_unique<CParameterMgrPlatformConnector>(configurationFilePath))
{
    std::string error;
    if (!mParameterMgrPlatformConnector->start(error))
    {
        throw Exception("Parameter framework fails to start : " + error);
    }
}

ParameterSerializer::~ParameterSerializer() {}

std::unique_ptr<CElementHandle> ParameterSerializer::getElement(
    const std::string &subsystemName,
    const std::string &moduleName,
    ParameterKind parameterKind) const
{
    std::string error;

    std::string rootElementName =
        mParameterMgrPlatformConnector->createElementHandle("/", error)->getName();
    if (rootElementName == "") {
        throw Exception(
            "No root element name found");
    }

    // compute module control element path from URL
    std::string moduleControlPath = std::string("/") + rootElementName
        + "/" + subsystemName + "/categories/" + moduleName + "/"
        + parameterKindHelper().toString(parameterKind) + "/";

    std::unique_ptr<CElementHandle> moduleElementHandle(
        mParameterMgrPlatformConnector->createElementHandle(moduleControlPath, error));

    if (moduleElementHandle == nullptr) {
        throw Exception(
            "Invalid parameters format: node for path \"" + moduleControlPath + "\" not found");
    }

    return moduleElementHandle;
}

std::unique_ptr<CElementHandle> ParameterSerializer::getChildElementHandle(
    const std::string &subsystemName,
    const std::string &elementName,
    ParameterKind parameterKind,
    const std::string &parameterName) const
{

    std::unique_ptr<CElementHandle> elementHandle = getElement(
        subsystemName, elementName, parameterKind);

    std::string error;
    std::unique_ptr<CElementHandle> childElementHandle(
        mParameterMgrPlatformConnector->createElementHandle(
        elementHandle->getPath() + "/" + parameterName, error));

    if (childElementHandle == nullptr) {
        throw Exception(
            "Child " + parameterName + " not found for " + elementHandle->getPath());
    }

    return childElementHandle;
}

void ParameterSerializer::stripFirstLine(std::string &document)
{
    std::size_t endLinePos = document.find("\n");
    assert(endLinePos != std::string::npos);
    document.erase(0, endLinePos + 1);
}

std::map<uint32_t, std::string>  ParameterSerializer::getChildren(
    const std::string &subsystemName,
    const std::string &elementName,
    ParameterKind parameterKind) const
{
    std::unique_ptr<CElementHandle> elementHandle = getElement(
        subsystemName, elementName, parameterKind);

    std::map<uint32_t, std::string> children;
    for (uint32_t childId = 0; childId < elementHandle->getChildrenCount(); childId++)
    {
        std::string childName;
        if (!elementHandle->getChildName(childId, childName)) {
            throw Exception("Child name not found for childId=" + std::to_string(childId));
        }
        children[childId] = childName;
    }
    return children;
}

std::vector<uint8_t> ParameterSerializer::xmlToBinary(
    const std::string &subsystemName,
    const std::string &elementName,
    ParameterKind parameterKind,
    const std::string &parameterName,
    const std::string parameterAsXml) const
{
    std::unique_ptr<CElementHandle> childElementHandle =
        getChildElementHandle(subsystemName, elementName, parameterKind, parameterName);

    // Send XML string to PFW
    std::string error;
    if (!childElementHandle->setAsXML(parameterAsXml, error))
    {
        throw Exception(
            "Not able to set XML stream for " + childElementHandle->getPath() + " : " + error);
    }

    // Read binary back from PFW
    return childElementHandle->getAsBytes();
}

std::string ParameterSerializer::binaryToXml(
    const std::string &subsystemName,
    const std::string &elementName,
    ParameterKind parameterKind,
    const std::string &parameterName,
    const std::vector<uint8_t> &parameterPayload) const
{
    std::unique_ptr<CElementHandle> childElementHandle =
        getChildElementHandle(subsystemName, elementName, parameterKind, parameterName);

    // Send binary to PFW
    std::string error;
    if (!childElementHandle->setAsBytes(parameterPayload, error))
    {
        throw Exception(
            "Not able to set payload for " + childElementHandle->getName() + " : " + error);
    }
    std::string result = childElementHandle->getAsXML();
    // Remove first line which is XML document header
    stripFirstLine(result);
    return result;
}

std::string ParameterSerializer::getStructureXml(
    const std::string &subsystemName,
    const std::string &elementName,
    ParameterKind parameterKind,
    const std::string &parameterName) const
{
    std::unique_ptr<CElementHandle> childElementHandle =
        getChildElementHandle(subsystemName, elementName, parameterKind, parameterName);

    std::string result = childElementHandle->getStructureAsXML();
    // Remove first line which is XML document header
    stripFirstLine(result);
    return result;
}

}
}
