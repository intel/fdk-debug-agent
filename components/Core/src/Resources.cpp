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
#include "Core/Resources.hpp"
#include "Core/InstanceModelConverter.hpp"
#include "Util/Uuid.hpp"
#include "Util/convert.hpp"
#include "IfdkObjects/Xml/TypeDeserializer.hpp"
#include "IfdkObjects/Xml/TypeSerializer.hpp"
#include "IfdkObjects/Xml/InstanceDeserializer.hpp"
#include "IfdkObjects/Xml/InstanceSerializer.hpp"
#include "Util/StringHelper.hpp"
#include <Poco/NumberParser.h>
#include <Poco/StringTokenizer.h>
#include <Poco/XML/XML.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/Node.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Text.h>
#include <Poco/DOM/AutoPtr.h>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

using namespace debug_agent::rest;
using namespace debug_agent::cavs;
using namespace debug_agent::ifdk_objects;

namespace debug_agent
{
namespace core
{

static const std::string ContentTypeHtml("text/html");
static const std::string ContentTypeXml("text/xml");
/**
* @fixme use the content type specified by SwAS. Currently, the SwAS does not specify
* which content type shall be used. A request has been sent to get SwAS updated. Until that,
* the rational is:
*  - the content is application specific: usage of 'application/'
*  - the content type is vendor specific: usage of standard 'vnd.'
*  - knowing the resource is an IFDK file, the client can read the streamed IFDK header to
*    know which <subsystem>:<file format> the resource is.
* @remarks http://www.iana.org/assignments/media-types/media-types.xhtml
*/
static const std::string ContentTypeBin("application/vnd.ifdk-file");

/** This method returns the value of a node part of an XML document, based on an XPath expression
 *
 *  @param const Poco::XML::Document* the XML document to parse
 *  @param const std::string& a simplified XPath expression describing the location of the node
 *  in the XML tree
 *
 *  @returns a std::string corresponding to the value of an XML node
 */
static const std::string getNodeValueFromXPath(const Poco::XML::Document* document,
                                                const std::string& url) {
    Poco::XML::Node* node = document->getNodeByPath(url);
    if (node) {
        return Poco::XML::fromXMLString(node->innerText());
    }
    else
    {
        throw debug_agent::rest::Resource::HttpError (
            debug_agent::rest::Resource::ErrorStatus::BadRequest,
            "Invalid parameters format: node for path \"" + url + "\" not found");
    }
}

void SystemTypeResource::handleGet(const Request &request, Response &response)
{
    xml::TypeSerializer serializer;
    mTypeModel.getSystem()->accept(serializer);
    std::string xml = serializer.getXml();

    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void SystemInstanceResource::handleGet(const Request &request, Response &response)
{
    xml::InstanceSerializer serializer;
    {
        ExclusiveInstanceModel::HandlePtr handle = mInstanceModel.acquireResource();
        if (handle->getResource() == nullptr) {
            throw HttpError(Resource::ErrorStatus::InternalError, "Instance model is undefined.");
        }
        assert(handle->getResource()->getSystem() != nullptr);
        handle->getResource()->getSystem()->accept(serializer);
    }

    std::string xml = serializer.getXml();
    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void TypeResource::handleGet(const Request &request, Response &response)
{
    std::string typeName = request.getIdentifierValue("type_name");
    std::shared_ptr<type::Type> typePtr =
        mTypeModel.getType(typeName);

    if (typePtr == nullptr) {
        throw HttpError(Resource::ErrorStatus::BadRequest, "Uknown type: " + typeName);
    }

    xml::TypeSerializer serializer;
    typePtr->accept(serializer);

    std::string xml = serializer.getXml();

    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void InstanceCollectionResource::handleGet(const Request &request, Response &response)
{
    xml::InstanceSerializer serializer;
    std::string typeName = request.getIdentifierValue("type_name");

    {
        ExclusiveInstanceModel::HandlePtr handle = mInstanceModel.acquireResource();
        if (handle->getResource() == nullptr) {
            throw HttpError(Resource::ErrorStatus::InternalError, "Instance model is undefined.");
        }
        std::shared_ptr<instance::BaseCollection> collection =
            handle->getResource()->getCollection(typeName);

        /* check nullptr using get() to avoid any KW error */
        if (collection.get() == nullptr) {
            throw HttpError(Resource::ErrorStatus::BadRequest, "Uknown type: " + typeName);
        }

        collection->accept(serializer);
    }

    std::string xml = serializer.getXml();
    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void InstanceResource::handleGet(const Request &request, Response &response)
{
    xml::InstanceSerializer serializer;
    std::string typeName = request.getIdentifierValue("type_name");
    std::string instanceId = request.getIdentifierValue("instance_id");

    {
        ExclusiveInstanceModel::HandlePtr handle = mInstanceModel.acquireResource();
        if (handle->getResource() == nullptr) {
            throw HttpError(Resource::ErrorStatus::InternalError, "Instance model is undefined.");
        }
        std::shared_ptr<instance::Instance> instancePtr =
            handle->getResource()->getInstance(typeName, instanceId);

        /* check nullptr using get() to avoid any KW error */
        if (instancePtr.get() == nullptr) {
            throw HttpError(Resource::ErrorStatus::BadRequest, "Uknown instance: type=" +
                typeName + " instance_id=" + instanceId);
        }

        instancePtr->accept(serializer);
    }

    std::string xml = serializer.getXml();
    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}


void RefreshSubsystemResource::handlePost(const Request &request, Response &response)
{
    std::shared_ptr<InstanceModel> instanceModel;
    try
    {
        InstanceModelConverter converter(mSystem);
        instanceModel = converter.createModel();
    }
    catch (BaseModelConverter::Exception &e)
    {
        throw HttpError(debug_agent::rest::Resource::ErrorStatus::InternalError,
            "Cannot refresh instance model: " + std::string(e.what()));
    }


    {
        ExclusiveInstanceModel::HandlePtr handle = mInstanceModel.acquireResource();
        handle->getResource() = instanceModel;
    }

    response.send(ContentTypeXml);
}

ControlParametersModuleInstanceResource::ControlParametersModuleInstanceResource(
    cavs::System &system,
    CParameterMgrPlatformConnector &parameterMgrPlatformConnector,
    const std::string &moduleName,
    const uint16_t moduleId) :
    SystemResource(system),
    mParameterMgrPlatformConnector(parameterMgrPlatformConnector),
    mModuleName(moduleName),
    mModuleId(moduleId)
{
    std::string error;
    if (!mParameterMgrPlatformConnector.isStarted())
    {
        if (!mParameterMgrPlatformConnector.start(error))
        {
            throw debug_agent::rest::Resource::HttpError(
                debug_agent::rest::Resource::ErrorStatus::InternalError,
                "Parameter framework fails to start :" + error);
        }
    }
}

CElementHandle *ControlParametersModuleInstanceResource::getModuleControlElement() const
{
    std::string error;
    std::string rootElementName = 
        mParameterMgrPlatformConnector.createElementHandle("/", error)->getName();

    // compute module control element path from URL
    std::string moduleControlPath = std::string("/") + rootElementName
        + "/cavs/categories/" + mModuleName + "/control/";

    CElementHandle *mModuleElementHandle =
        mParameterMgrPlatformConnector.createElementHandle(moduleControlPath, error);

    if (mModuleElementHandle == nullptr) {
        throw debug_agent::rest::Resource::HttpError(
            debug_agent::rest::Resource::ErrorStatus::NotFound,
            "Invalid parameters format: node for path \"" + moduleControlPath + "\" not found");
    }

    return mModuleElementHandle;
}

CElementHandle *ControlParametersModuleInstanceResource::getChildElementHandle(
    const CElementHandle *moduleElementHandle, uint32_t childId) const
{
    std::string childName;
    if (!moduleElementHandle->getChildName(childId, childName)) {
        throw rest::Resource::HttpError(
            rest::Resource::ErrorStatus::BadRequest,
            "Child name not found for childId=" + std::to_string(childId));
    }

    std::string error;
    CElementHandle* childElementHandle =
        mParameterMgrPlatformConnector.createElementHandle(
        moduleElementHandle->getPath() + "/" + childName, error);
    if (childElementHandle == nullptr) {
        throw rest::Resource::HttpError(
            rest::Resource::ErrorStatus::BadRequest,
            "Child " + childName + " not found for " + moduleElementHandle->getName());
    }

    return childElementHandle;
}

uint32_t ControlParametersModuleInstanceResource::getElementMapping(
    const CElementHandle *elementHandle) const
{
    std::string paramIdAsString;
    if (!elementHandle->getMappingData("ParamId", paramIdAsString))
    {
        throw rest::Resource::HttpError(
            rest::Resource::ErrorStatus::BadRequest,
            "Mapping \"ParamId\" not found for " + elementHandle->getName());
    }
    uint32_t paramId;
    convertTo(paramIdAsString, paramId);

    return paramId;
}

void ControlParametersModuleInstanceResource::handleGet(const Request &request, Response &response)
{
    /* Checking that the identifiers has been fetched */
    uint16_t instanceId;
    convertTo(request.getIdentifierValue("instanceId"), instanceId);

    std::ostream &out = response.send(ContentTypeXml);
    out << "<control_parameters Type=\"module-";
    out << mModuleName;
    out << "\" Id=\"";
    out << mModuleId;
    out << "\">\n";

    CElementHandle *moduleElementHandle = getModuleControlElement();

    /* Loop through children to get Settings */
    std::string controlParameters;
    uint32_t childrenCount = moduleElementHandle->getChildrenCount();
    for (uint32_t child = 0; child < childrenCount; child++)
    {
        CElementHandle* childElementHandle = getChildElementHandle(moduleElementHandle, child);

        uint32_t paramId = getElementMapping(childElementHandle);

        // Get binary from IOCTL
        std::vector<uint8_t> parameterPayload;
        mSystem.getModuleParameter(mModuleId, instanceId, paramId, parameterPayload);
        // Send binary to PFW
        std::string error;
        if (!childElementHandle->setAsBytes(parameterPayload, error))
        {
            throw rest::Resource::HttpError(
                rest::Resource::ErrorStatus::BadRequest,
                "Not able to set payload for " + childElementHandle->getName() + " : " + error);
        }
        //childElementHandle->setAsBytes()
        std::string result = childElementHandle->getAsXML();
        // Remove first line which is XML document header
        controlParameters += result.erase(0, result.find("\n") + 1);
    }

    out << controlParameters;
    out << "</control_parameters>";
}

void ControlParametersModuleInstanceResource::handlePut(const Request &request, Response &response)
{
    std::string error;
    Poco::XML::DOMParser parser;
    Poco::XML::Document* document = parser.parseString(request.getRequestContentAsString());
    static const std::string controlParametersUrl = "/control_parameters/";

    /* Checking that the identifiers has been fetched */
    uint16_t instanceId;
    convertTo(request.getIdentifierValue("instanceId"), instanceId);

    CElementHandle *moduleElementHandle = getModuleControlElement();

    /* Loop through children to get Settings */
    uint32_t childrenCount = moduleElementHandle->getChildrenCount();
    for (uint32_t child = 0; child < childrenCount; child++)
    {
        CElementHandle* childElementHandle = getChildElementHandle(moduleElementHandle, child);

        uint32_t paramId = getElementMapping(childElementHandle);

        /* Create XML document from the XML node of each child */
        Poco::XML::AutoPtr<Poco::XML::Document> childDocument(new Poco::XML::Document);
        childDocument->appendChild(childDocument->importNode(document->getNodeByPath(
            controlParametersUrl + "ParameterBlock[@Name='" + childElementHandle->getName() + "']"), true));
        /* Serialize the document in a stream*/
        Poco::XML::DOMWriter writer;
        std::stringstream output;
        writer.writeNode(output, childDocument);

        // Send XML string to PFW
        if (!childElementHandle->setAsXML(output.str(), error))
        {
            throw rest::Resource::HttpError(
                rest::Resource::ErrorStatus::InternalError,
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

void SubsystemInstanceLogParametersResource::handleGet(const Request &request, Response &response)
{
    Logger::Parameters logParameters;

    try
    {
        logParameters = mSystem.getLogParameters();
    }
    catch (System::Exception &e)
    {
        throw HttpError(ErrorStatus::BadRequest, "Cannot get log parameters : " +
            std::string(e.what()));
    }

    std::ostream &out = response.send(ContentTypeXml);
    out << "<service Direction=\"Outgoing\" Type=\"fwlogs\" Id=\"0\">\n"
        "    <parents/>\n"
        "    <control_parameters>\n"
        "        <BooleanParameter Name=\"Started\">" <<
                    logParameters.mIsStarted << "</BooleanParameter>\n"
        "        <ParameterBlock Name=\"Buffering\">\n"
        "            <IntegerParameter Name=\"Size\">100</IntegerParameter>\n"
        "            <BooleanParameter Name=\"Circular\">0</BooleanParameter>\n"
        "        </ParameterBlock>\n"
        "        <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>\n"
        "        <EnumParameter Name=\"Verbosity\">" <<
                    Logger::toString(logParameters.mLevel) << "</EnumParameter>\n"
        "        <BooleanParameter Name=\"ViaPTI\">" <<
                    (logParameters.mOutput == Logger::Output::Pti ? 1 : 0)
                    << "</BooleanParameter>\n"
        "    </control_parameters>\n"
        "</service>\n";
}

void SubsystemInstanceLogParametersResource::handlePut(const Request &request, Response &response)
{
    Poco::XML::DOMParser parser;
    Poco::XML::Document* document = parser.parseString(request.getRequestContentAsString());

    if (!document) {
        throw HttpError(ErrorStatus::BadRequest, "Invalid document");
    }

    static const std::string controlParametersUrl = "/service[@Type='fwlogs']/control_parameters/";

    // Retrieve the Started BooleanParameter and its value
    std::string startedNodeValue = getNodeValueFromXPath(document,
        controlParametersUrl + "BooleanParameter[@Name='Started']");

    // Retrieve the Verbosity EnumParameter and its value
    std::string verbosityNodeValue = getNodeValueFromXPath(document,
        controlParametersUrl + "EnumParameter[@Name='Verbosity']");

    // Retrieve the ViaPTI BooleanParameter and its value
    std::string viaPtiNodeValue = getNodeValueFromXPath(document,
        controlParametersUrl + "BooleanParameter[@Name='ViaPTI']");

    // Parse each of the parameters found into their correct type
    Logger::Parameters logParameters;
    try {
        logParameters.mIsStarted =
            Poco::NumberParser::parseBool(startedNodeValue);
        logParameters.mLevel =
            Logger::levelFromString(verbosityNodeValue);
        logParameters.mOutput =
            Poco::NumberParser::parseBool(viaPtiNodeValue) ? Logger::Output::Pti :
                                                             Logger::Output::Sram;
    }
    catch (Logger::Exception &e) {
        throw HttpError(ErrorStatus::BadRequest, std::string("Invalid value: ") + e.what());
    }
    catch (Poco::SyntaxException &e) {
        throw HttpError(ErrorStatus::BadRequest,
            std::string("Invalid Start/Stop request: ") + e.what());
    }

    try {
        mSystem.setLogParameters(logParameters);
    }
    catch (System::Exception &e) {
        throw HttpError(ErrorStatus::BadRequest, std::string("Fail to apply: ") + e.what());
    }

    std::ostream &out = response.send(ContentTypeHtml);
    out << "<p>Done</p>";
}

void SubsystemInstanceLogControlParametersResource::handleGet(const Request &request, Response &response)
{
    Logger::Parameters logParameters;

    try
    {
        logParameters = mSystem.getLogParameters();
    }
    catch (System::Exception &e)
    {
        throw HttpError(ErrorStatus::BadRequest, "Cannot get log parameters : " +
            std::string(e.what()));
    }

    std::ostream &out = response.send(ContentTypeXml);
    out << "<control_parameters>\n"
        "    <BooleanParameter Name=\"Started\">" <<
        logParameters.mIsStarted << "</BooleanParameter>"
        "    <ParameterBlock Name=\"Buffering\">\n"
        "        <IntegerParameter Name=\"Size\">0</IntegerParameter>\n"
        "        <BooleanParameter Name=\"Circular\">0</BooleanParameter>\n"
        "    </ParameterBlock>\n"
        "    <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>\n"
        "    <EnumParameter Name=\"Verbosity\">" <<
        Logger::toString(logParameters.mLevel) << "</EnumParameter>\n"
        "    <BooleanParameter Name=\"ViaPTI\">" <<
        (logParameters.mOutput == Logger::Output::Pti ? 1 : 0) << "</BooleanParameter>\n"
        "</control_parameters>\n";
}

void SubsystemInstanceLogControlParametersResource::handlePut(const Request &request, Response &response)
{
    Poco::XML::DOMParser parser;
    Poco::XML::Document* document = parser.parseString(request.getRequestContentAsString());

    if (!document) {
        throw HttpError(ErrorStatus::BadRequest, "Invalid document");
    }

    static const std::string controlParametersUrl = "/control_parameters/";

    // Retrieve the Started BooleanParameter and its value
    std::string startedNodeValue = getNodeValueFromXPath(document,
        controlParametersUrl + "BooleanParameter[@Name='Started']");

    // Retrieve the Verbosity EnumParameter and its value
    std::string verbosityNodeValue = getNodeValueFromXPath(document,
        controlParametersUrl + "EnumParameter[@Name='Verbosity']");

    // Retrieve the ViaPTI BooleanParameter and its value
    std::string viaPtiNodeValue = getNodeValueFromXPath(document,
        controlParametersUrl + "BooleanParameter[@Name='ViaPTI']");

    // Parse each of the parameters found into their correct type
    Logger::Parameters logParameters;
    try {
        logParameters.mIsStarted =
            Poco::NumberParser::parseBool(startedNodeValue);
        logParameters.mLevel =
            Logger::levelFromString(verbosityNodeValue);
        logParameters.mOutput =
            Poco::NumberParser::parseBool(viaPtiNodeValue) ? Logger::Output::Pti :
            Logger::Output::Sram;
    }
    catch (Logger::Exception &e) {
        throw HttpError(ErrorStatus::BadRequest, std::string("Invalid value: ") + e.what());
    }
    catch (Poco::SyntaxException &e) {
        throw HttpError(ErrorStatus::BadRequest,
            std::string("Invalid Start/Stop request: ") + e.what());
    }

    try {
        mSystem.setLogParameters(logParameters);
    }
    catch (System::Exception &e) {
        throw HttpError(ErrorStatus::InternalError, std::string("Fail to apply: ") + e.what());
    }

    std::ostream &out = response.send(ContentTypeHtml);
    out << "<p>Done</p>";
}

void SubsystemTypeLogParametersResource::handleGet(const Request &request, Response &response)
{
    std::ostream &out = response.send(ContentTypeXml);
    out << "<service_type Name=\"fwlogs\">\n"
        "    <control_parameters>\n"
        "        <!-- service generic -->\n"
        "        <BooleanParameter Name=\"Started\"/>\n"
        "        <ParameterBlock Name=\"Buffering\">\n"
        "            <IntegerParameter Name=\"Size\" Size=\"16\" Unit=\"MegaBytes\"/>\n"
        "            <BooleanParameter Name=\"Circular\"/>\n"
        "        </ParameterBlock>\n"
        "        <BooleanParameter Name=\"PersistsState\"/>\n"
        "        <!-- service specific -->\n"
        "        <EnumParameter Size=\"8\" Name=\"Verbosity\">\n"
        "            <ValuePair Numerical=\"2\" Literal=\"Critical\"/>\n"
        "            <ValuePair Numerical=\"3\" Literal=\"High\"/>\n"
        "            <ValuePair Numerical=\"4\" Literal=\"Medium\"/>\n"
        "            <ValuePair Numerical=\"5\" Literal=\"Low\"/>\n"
        "            <ValuePair Numerical=\"6\" Literal=\"Verbose\"/>\n"
        "        </EnumParameter>\n"
        "        <BooleanParameter Name=\"ViaPTI\" " <<
                                  "Description=\"Set to 1 if PTI interface is to be used\"/>\n"
        "    </control_parameters>\n"
        "</service_type>\n";
}

void SubsystemInstanceLogStreamResource::handleGet(const Request &request, Response &response)
{
    /** Acquiring the log stream resource */
    auto resource = std::move(mSystem.tryToAcquireLogStreamResource());
    if (resource == nullptr) {
        throw HttpError(ErrorStatus::Locked, "Logging stream resource is already used.");
    }

    std::ostream &out = response.send(ContentTypeBin);

    try {
        resource->doLogStream(out);
    }
    catch (System::Exception &e)
    {
        /* Here a successful http response has already be sent to the server. So
        * it is not possible to throw the exception */

        /** @todo use logging */
        std::cout << "Exception while getting logs from system: " << e.what();
    }
}

}
}
