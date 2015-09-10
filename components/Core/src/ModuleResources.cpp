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
#include "Core/ModuleResources.hpp"
#include "Util/Uuid.hpp"
#include "Util/convert.hpp"
#include <Poco/NumberParser.h>
#include <Poco/StringTokenizer.h>
#include <Poco/XML/XML.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/Node.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/AutoPtr.h>
#include <cassert>

using namespace debug_agent::rest;
using namespace debug_agent::cavs;

namespace debug_agent
{
namespace core
{
    static const std::string ContentTypeHtml("text/html");
    static const std::string ContentTypeXml("text/xml");

PFWResource::PFWResource(cavs::System &system,
    CParameterMgrPlatformConnector &parameterMgrPlatformConnector) :
    SystemResource(system),
    mParameterMgrPlatformConnector(parameterMgrPlatformConnector)
{
    std::string error;
    if (!mParameterMgrPlatformConnector.isStarted())
    {
        if (!mParameterMgrPlatformConnector.start(error))
        {
            throw Resource::HttpError(
                Resource::ErrorStatus::InternalError,
                "Parameter framework fails to start : " + error);

        }
    }
}

std::unique_ptr<CElementHandle> ModuleResource::getModuleControlElement()
    const
{
    std::string error;
    std::string rootElementName =
        getParameterMgrPlatformConnector().createElementHandle("/", error)->getName();

    // compute module control element path from URL
    std::string moduleControlPath = std::string("/") + rootElementName
        + "/cavs/categories/" + mModuleName + "/control/";

    std::unique_ptr<CElementHandle> mModuleElementHandle(
        getParameterMgrPlatformConnector().createElementHandle(moduleControlPath, error));

    if (mModuleElementHandle == nullptr) {
        throw Resource::HttpError(
            Resource::ErrorStatus::NotFound,
            "Invalid parameters format: node for path \"" + moduleControlPath + "\" not found");
    }

    return mModuleElementHandle;
}

std::unique_ptr<CElementHandle> ModuleResource::getChildElementHandle(
    const CElementHandle &moduleElementHandle, uint32_t childId) const
{
    std::string childName;
    if (!moduleElementHandle.getChildName(childId, childName)) {
        throw Resource::HttpError(
            Resource::ErrorStatus::BadRequest,
            "Child name not found for childId=" + std::to_string(childId));
    }

    std::string error;
    std::unique_ptr<CElementHandle> childElementHandle(
        getParameterMgrPlatformConnector().createElementHandle(
            moduleElementHandle.getPath() + "/" + childName, error));

    if (childElementHandle == nullptr) {
        throw Resource::HttpError(
            Resource::ErrorStatus::BadRequest,
            "Child " + childName + " not found for " + moduleElementHandle.getName());
    }

    return childElementHandle;
}

uint32_t ModuleResource::getElementMapping(
    const CElementHandle &elementHandle) const
{
    std::string paramIdAsString;
    if (!elementHandle.getMappingData("ParamId", paramIdAsString))
    {
        throw Resource::HttpError(
            Resource::ErrorStatus::BadRequest,
            "Mapping \"ParamId\" not found for " + elementHandle.getName());
    }
    uint32_t paramId;
    if (!convertTo(paramIdAsString, paramId)) {

        throw Resource::HttpError(
            Resource::ErrorStatus::InternalError,
            "Invalid mapping \"ParamId\": " + paramIdAsString);
    }

    return paramId;
}

void ControlParametersModuleInstanceResource::handleGet(const Request &request, Response &response)
{
    /* Checking that the identifiers has been fetched */
    uint16_t instanceId;
    std::string instanceIdValue(request.getIdentifierValue("instanceId"));
    if (!convertTo(instanceIdValue, instanceId)) {

        throw Resource::HttpError(
            Resource::ErrorStatus::BadRequest,
            "Invalid instance ID: " + instanceIdValue);
    }

    std::unique_ptr<CElementHandle> moduleElementHandle = getModuleControlElement();

    /* Loop through children to get Settings */
    std::string controlParameters;
    uint32_t childrenCount = moduleElementHandle->getChildrenCount();
    for (uint32_t child = 0; child < childrenCount; child++)
    {
        std::unique_ptr<CElementHandle> childElementHandle =
            getChildElementHandle(*moduleElementHandle, child);

        uint32_t paramId = getElementMapping(*childElementHandle);

        // Get binary from IOCTL
        std::vector<uint8_t> parameterPayload;
        mSystem.getModuleParameter(mModuleId, instanceId, paramId, parameterPayload);
        // Send binary to PFW
        std::string error;
        if (!childElementHandle->setAsBytes(parameterPayload, error))
        {
            throw Resource::HttpError(
                Resource::ErrorStatus::BadRequest,
                "Not able to set payload for " + childElementHandle->getName() + " : " + error);
        }
        //childElementHandle->setAsBytes()
        std::string result = childElementHandle->getAsXML();
        // Remove first line which is XML document header
        std::size_t endLinePos = result.find("\n");
        assert(endLinePos != std::string::npos);
        controlParameters += result.erase(0, endLinePos + 1);
    }

    std::ostream &out = response.send(ContentTypeXml);
    out << "<control_parameters Type=\"module-";
    out << mModuleName;
    out << "\" Id=\"";
    out << mModuleId;
    out << "\">\n";
    out << controlParameters;
    out << "</control_parameters>\n";
}

void ControlParametersModuleInstanceResource::handlePut(const Request &request, Response &response)
{
    std::string error;
    Poco::XML::DOMParser parser;
    Poco::XML::AutoPtr<Poco::XML::Document> document =
        parser.parseString(request.getRequestContentAsString());
    static const std::string controlParametersUrl = "/control_parameters/";

    /* Checking that the identifiers has been fetched */
    uint16_t instanceId;
    std::string instanceIdValue(request.getIdentifierValue("instanceId"));
    if (!convertTo(instanceIdValue, instanceId)) {

        throw Resource::HttpError(
            Resource::ErrorStatus::BadRequest,
            "Invalid instance ID: " + instanceIdValue);
    }

    std::unique_ptr<CElementHandle> moduleElementHandle = getModuleControlElement();

    /* Loop through children to get Settings */
    uint32_t childrenCount = moduleElementHandle->getChildrenCount();
    for (uint32_t child = 0; child < childrenCount; child++)
    {
        std::unique_ptr<CElementHandle> childElementHandle =
            getChildElementHandle(*moduleElementHandle, child);

        uint32_t paramId = getElementMapping(*childElementHandle);

        /* Create XML document from the XML node of each child */
        Poco::XML::AutoPtr<Poco::XML::Document> childDocument = new Poco::XML::Document;
        childDocument->appendChild(childDocument->importNode(
            document->getNodeByPath(controlParametersUrl + "ParameterBlock[@Name='"
            + childElementHandle->getName() + "']"), true));
        /* Serialize the document in a stream*/
        Poco::XML::DOMWriter writer;
        std::stringstream output;
        writer.writeNode(output, childDocument);

        // Send XML string to PFW
        if (!childElementHandle->setAsXML(output.str(), error))
        {
            throw Resource::HttpError(
                Resource::ErrorStatus::BadRequest,
                "Not able to set XML stream for " + childElementHandle->getName() + " : " + error);
        }
        // Read binary back from PFW
        std::vector<uint8_t> parameterPayload = childElementHandle->getAsBytes();
        // Send binary to IOCTL
        mSystem.setModuleParameter(mModuleId, instanceId, paramId, parameterPayload);
    }
    std::ostream &out = response.send(ContentTypeHtml);
    out << "<p>Done</p>";
}

void ControlParametersModuleTypeResource::handleGet(const Request &request, Response &response)
{
    /* Checking that the identifiers has been fetched */
    uint16_t instanceId;
    std::string instanceIdValue(request.getIdentifierValue("instanceId"));
    if (!convertTo(instanceIdValue, instanceId)) {

        throw Resource::HttpError(
            Resource::ErrorStatus::BadRequest,
            "Invalid instance ID: " + instanceIdValue);
    }

    std::unique_ptr<CElementHandle> moduleElementHandle = getModuleControlElement();

    /* Loop through children to get Settings */
    std::string controlParameters;
    uint32_t childrenCount = moduleElementHandle->getChildrenCount();
    for (uint32_t child = 0; child < childrenCount; child++)
    {
        std::unique_ptr<CElementHandle>  childElementHandle = getChildElementHandle(
            *moduleElementHandle, child);

        // Get Structure information from PFW
        std::string result = childElementHandle->getStructureAsXML();
        // Remove first line which is XML document header
        std::size_t endLinePos = result.find("\n");
        assert(endLinePos != std::string::npos);
        controlParameters += result.erase(0, endLinePos + 1);
    }

    std::ostream &out = response.send(ContentTypeXml);
    out << "<control_parameters Type=\"module-";
    out << mModuleName;
    out << "\" Id=\"";
    out << mModuleId;
    out << "\">\n";
    out << controlParameters;
    out << "</control_parameters>\n";
}

}
}
