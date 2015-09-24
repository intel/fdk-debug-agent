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
#include "Rest/StreamResponse.hpp"
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
#include <sstream>
#include <cassert>

using namespace debug_agent::rest;
using namespace debug_agent::cavs;
using namespace debug_agent::parameterSerializer;

namespace debug_agent
{
namespace core
{
    static const std::string ContentTypeHtml("text/html");
    static const std::string ContentTypeXml("text/xml");
    static const std::string InstanceId("instanceId");
    static const std::string ParamId("ParamId");

std::map<uint32_t, std::string> ModuleResource::getChildren(
    ParameterSerializer::ParameterKind parameterKind) const
{
    std::map<uint32_t, std::string>  children;
    try
    {
        children = mParameterSerializer.getChildren(
            "cavs", mModuleName, parameterKind);
    }
    catch (ParameterSerializer::Exception &e)
    {
        throw Response::HttpError(
            Response::ErrorStatus::InternalError,
            "Module resource does not manage to get children: " + std::string(e.what()));
    }
    return children;
}

uint16_t ModuleResource::getInstanceId(const Request &request) const
{
    uint16_t instanceId;
    std::string instanceIdValue(request.getIdentifierValue(InstanceId));
    if (!convertTo(instanceIdValue, instanceId)) {

        throw Response::HttpError(
            Response::ErrorStatus::BadRequest,
            "Invalid instance ID: " + instanceIdValue);
    }
    return instanceId;
}

uint32_t ModuleResource::getParamId(const std::string parameterName) const
{
    uint32_t paramId;
    std::string paramIdAsString;
    try
    {
        paramIdAsString = mParameterSerializer.getMapping(
            "cavs", mModuleName, parameterName, ParamId);
    }
    catch (ParameterSerializer::Exception &e)
    {
        throw Response::HttpError(
            Response::ErrorStatus::InternalError,
            "Module resource does not manage to retrieve mapping: " + std::string(e.what()));
    }

    if (!convertTo(paramIdAsString, paramId)) {

        throw Response::HttpError(
            Response::ErrorStatus::InternalError,
            "Invalid mapping \"ParamId\": " + paramIdAsString);
    }

    return paramId;
}

Resource::ResponsePtr ControlParametersModuleInstanceResource::handleGet(
    const Request &request)
{
    /* Checking that the identifiers has been fetched */
    uint16_t instanceId = getInstanceId(request);

    /* Loop through children to get Settings */
    std::map<uint32_t, std::string>  children = getChildren(
        ParameterSerializer::ParameterKind::Control);

    std::string controlParameters;
    for (uint32_t childId = 0; childId < children.size(); childId++)
    {
        uint32_t paramId = getParamId(children[childId]);

        // Get binary from IOCTL
        std::vector<uint8_t> parameterPayload;
        try
        {
            mSystem.getModuleParameter(mModuleId, instanceId, paramId, parameterPayload);
        }
        catch (ModuleHandler::Exception &e)
        {
            throw Response::HttpError(
                Response::ErrorStatus::InternalError,
                "Cannot get module parameter: " + std::string(e.what()));
        }

        // Convert to XML
        try
        {
            controlParameters += mParameterSerializer.binaryToXml(
                "cavs",
                mModuleName,
                ParameterSerializer::ParameterKind::Control,
                children[childId],
                parameterPayload);
        }
        catch (ParameterSerializer::Exception &e)
        {
            throw Response::HttpError(
                Response::ErrorStatus::InternalError,
                "Binary to Xml conversion failed: " + std::string(e.what()));
        }
    }

    auto out = std::make_unique<std::stringstream>();
    *out << "<control_parameters Type=\"module-"
        << mModuleName
        << "\" Id=\""
        << mModuleId
        << "\">\n"
        << controlParameters
        << "</control_parameters>\n";

    return std::make_unique<StreamResponse>(ContentTypeXml, std::move(out));
}

Resource::ResponsePtr ControlParametersModuleInstanceResource::handlePut(
    const Request &request)
{
    std::string error;
    Poco::XML::DOMParser parser;
    Poco::XML::AutoPtr<Poco::XML::Document> document =
        parser.parseString(request.getRequestContentAsString());
    static const std::string controlParametersUrl = "/control_parameters/";

    /* Checking that the identifiers has been fetched */
    uint16_t instanceId = getInstanceId(request);

    std::map<uint32_t, std::string>  children = getChildren(
        ParameterSerializer::ParameterKind::Control);

    for (uint32_t childId = 0; childId < children.size(); childId++)
    {
        uint32_t paramId = getParamId(children[childId]);

        /* Create XML document from the XML node of each child. The usage of operator new is needed
         * here to comply with poco AutoPtr. */
        Poco::XML::AutoPtr<Poco::XML::Document> childDocument = new Poco::XML::Document;
        childDocument->appendChild(childDocument->importNode(
            document->getNodeByPath(controlParametersUrl + "ParameterBlock[@Name='"
            + children[childId] + "']"), true));
        /* Serialize the document in a stream*/
        Poco::XML::DOMWriter writer;
        std::stringstream output;
        writer.writeNode(output, childDocument);

        // Send XML string to PFW
        std::vector<uint8_t> parameterPayload;
        // Convert XML to binary
        try
        {
            parameterPayload = mParameterSerializer.xmlToBinary(
                "cavs",
                mModuleName,
                ParameterSerializer::ParameterKind::Control,
                children[childId],
                output.str());
        }
        catch (ParameterSerializer::Exception &e)
        {
            throw Response::HttpError(
                Response::ErrorStatus::InternalError,
                "Xml to binary conversion failed: " + std::string(e.what()));
        }
        // Send binary to IOCTL
        try
        {
            mSystem.setModuleParameter(mModuleId, instanceId, paramId, parameterPayload);
        }
        catch (ModuleHandler::Exception &e)
        {
            throw Response::HttpError(
                Response::ErrorStatus::InternalError,
                "Cannot set module parameter: " + std::string(e.what()));
        }
    }

    return std::make_unique<Response>();
}

Resource::ResponsePtr ControlParametersModuleTypeResource::handleGet(
    const Request &request)
{
    /* Checking that the identifiers has been fetched */
    uint16_t instanceId = getInstanceId(request);

    /* Loop through children to get Settings */
    std::map<uint32_t, std::string>  children = getChildren(
        ParameterSerializer::ParameterKind::Control);

    std::string controlParameters;
    for (uint32_t childId = 0; childId < children.size(); childId++)
    {
        try
        {
            controlParameters += mParameterSerializer.getStructureXml(
                "cavs",
                mModuleName,
                ParameterSerializer::ParameterKind::Control,
                children[childId]);
        }
        catch (ParameterSerializer::Exception &e)
        {
            throw Response::HttpError(
                Response::ErrorStatus::InternalError,
                "Failed to get Xml structure: " + std::string(e.what()));
        };
    }

    auto out = std::make_unique<std::stringstream>();
    *out << "<control_parameters Type=\"module-"
        << mModuleName
        << "\" Id=\""
        << mModuleId
        << "\">\n"
        << controlParameters
        << "</control_parameters>\n";

    return std::make_unique<StreamResponse>(ContentTypeXml, std::move(out));
}

}
}
