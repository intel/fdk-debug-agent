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
#include "Rest/CustomResponse.hpp"
#include "Rest/StreamResponse.hpp"
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
static const std::string ContentTypeIfdkFile("application/vnd.ifdk-file");

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
        throw Response::HttpError (
            Response::ErrorStatus::BadRequest,
            "Invalid parameters format: node for path \"" + url + "\" not found");
    }
}

Resource::ResponsePtr SystemTypeResource::handleGet(const Request &request)
{
    xml::TypeSerializer serializer;
    mTypeModel.getSystem()->accept(serializer);

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr SystemInstanceResource::handleGet(const Request &request)
{
    xml::InstanceSerializer serializer;
    mSystemInstance.accept(serializer);

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr TypeResource::handleGet(const Request &request)
{
    std::string typeName = request.getIdentifierValue("type_name");
    std::shared_ptr<const type::Type> typePtr =
        mTypeModel.getType(typeName);

    if (typePtr == nullptr) {
        throw Response::HttpError(Response::ErrorStatus::BadRequest, "Unknown type: " + typeName);
    }

    xml::TypeSerializer serializer;
    typePtr->accept(serializer);

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr InstanceCollectionResource::handleGet(const Request &request)
{
    xml::InstanceSerializer serializer;
    std::string typeName = request.getIdentifierValue("type_name");

    {
        ExclusiveInstanceModel::HandlePtr handle = mInstanceModel.acquireResource();
        if (handle->getResource() == nullptr) {
            throw Response::HttpError(Response::ErrorStatus::InternalError,
                "Instance model is undefined.");
        }
        std::shared_ptr<const instance::BaseCollection> collection =
            handle->getResource()->getCollection(typeName);

        /* check nullptr using get() to avoid any KW error */
        if (collection.get() == nullptr) {
            throw Response::HttpError(Response::ErrorStatus::BadRequest,
                "Unknown type: " + typeName);
        }

        collection->accept(serializer);
    }

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}

Resource::ResponsePtr InstanceResource::handleGet(const Request &request)
{
    xml::InstanceSerializer serializer;
    std::string typeName = request.getIdentifierValue("type_name");
    std::string instanceId = request.getIdentifierValue("instance_id");

    {
        ExclusiveInstanceModel::HandlePtr handle = mInstanceModel.acquireResource();
        if (handle->getResource() == nullptr) {
            throw Response::HttpError(Response::ErrorStatus::InternalError,
                "Instance model is undefined.");
        }
        std::shared_ptr<const instance::Instance> instancePtr =
            handle->getResource()->getInstance(typeName, instanceId);

        /* check nullptr using get() to avoid any KW error */
        if (instancePtr.get() == nullptr) {
            throw Response::HttpError(Response::ErrorStatus::BadRequest,
                "Unknown instance: type=" + typeName + " instance_id=" + instanceId);
        }

        instancePtr->accept(serializer);
    }

    return std::make_unique<Response>(ContentTypeXml, serializer.getXml());
}


Resource::ResponsePtr RefreshSubsystemResource::handlePost(const Request &request)
{
    std::shared_ptr<InstanceModel> instanceModel;
    ExclusiveInstanceModel::HandlePtr handle = mInstanceModel.acquireResource();

    try
    {
        InstanceModelConverter converter(mSystem);
        instanceModel = converter.createModel();
    }
    catch (BaseModelConverter::Exception &e)
    {
        /* Topology retrieving has failed: invalidate the previous one */
        handle->getResource() = nullptr;

        throw Response::HttpError(Response::ErrorStatus::InternalError,
            "Cannot refresh instance model: " + std::string(e.what()));
    }

    /* Apply new topology */
    handle->getResource() = instanceModel;

    return std::make_unique<Response>();
}

Resource::ResponsePtr LogServiceInstanceControlParametersResource::handleGet(const Request &request)
{
    Logger::Parameters logParameters;

    try
    {
        logParameters = mSystem.getLogParameters();
    }
    catch (System::Exception &e)
    {
        throw Response::HttpError(Response::ErrorStatus::BadRequest,
            std::string("Cannot get log parameters : ") + e.what());
    }

    auto xml = std::make_unique<std::stringstream>();
    *xml << "<control_parameters>\n"
        "    <BooleanParameter Name=\"Started\">" <<
        logParameters.mIsStarted << "</BooleanParameter>\n"
        "    <ParameterBlock Name=\"Buffering\">\n"
        "        <IntegerParameter Name=\"Size\">100</IntegerParameter>\n"
        "        <BooleanParameter Name=\"Circular\">0</BooleanParameter>\n"
        "    </ParameterBlock>\n"
        "    <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>\n"
        "    <EnumParameter Name=\"Verbosity\">" <<
        Logger::toString(logParameters.mLevel) << "</EnumParameter>\n"
        "    <BooleanParameter Name=\"ViaPTI\">" <<
        (logParameters.mOutput == Logger::Output::Pti ? 1 : 0) << "</BooleanParameter>\n"
        "</control_parameters>\n";

    return std::make_unique<StreamResponse>(ContentTypeXml, std::move(xml));
}

Resource::ResponsePtr LogServiceInstanceControlParametersResource::handlePut(const Request &request)
{
    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> document(
        parser.parseString(request.getRequestContentAsString()));

    if (!document) {
        throw Response::HttpError(Response::ErrorStatus::BadRequest, "Invalid document");
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
        throw Response::HttpError(Response::ErrorStatus::BadRequest,
            std::string("Invalid value: ") + e.what());
    }
    catch (Poco::SyntaxException &e) {
        throw Response::HttpError(Response::ErrorStatus::BadRequest,
            std::string("Invalid Start/Stop request: ") + e.what());
    }

    try {
        mSystem.setLogParameters(logParameters);
    }
    catch (System::Exception &e) {
        throw Response::HttpError(Response::ErrorStatus::InternalError,
            std::string("Fail to apply: ") + e.what());
    }

    return std::make_unique<Response>();
}

Resource::ResponsePtr LogServiceTypeControlParametersResource::handleGet(const Request &request)
{
    auto xml = std::make_unique<std::stringstream>();
    *xml <<
        "<control_parameters>\n"
        "    <!-- service generic -->\n"
        "    <BooleanParameter Name=\"Started\"/>\n"
        "    <ParameterBlock Name=\"Buffering\">\n"
        "        <IntegerParameter Name=\"Size\" Size=\"16\" Unit=\"MegaBytes\"/>\n"
        "        <BooleanParameter Name=\"Circular\"/>\n"
        "    </ParameterBlock>\n"
        "    <BooleanParameter Name=\"PersistsState\"/>\n"
        "    <!-- service specific -->\n"
        "    <EnumParameter Size=\"8\" Name=\"Verbosity\">\n"
        "        <ValuePair Numerical=\"2\" Literal=\"Critical\"/>\n"
        "        <ValuePair Numerical=\"3\" Literal=\"High\"/>\n"
        "        <ValuePair Numerical=\"4\" Literal=\"Medium\"/>\n"
        "        <ValuePair Numerical=\"5\" Literal=\"Low\"/>\n"
        "        <ValuePair Numerical=\"6\" Literal=\"Verbose\"/>\n"
        "    </EnumParameter>\n"
        "    <BooleanParameter Name=\"ViaPTI\" "
        "Description=\"Set to 1 if PTI interface is to be used\"/>\n"
        "</control_parameters>\n";

    return std::make_unique<StreamResponse>(ContentTypeXml, std::move(xml));
}

Resource::ResponsePtr LogServiceStreamResource::handleGet(const Request &request)
{
    /**
     * @todo Once C++14 is fully supported by compilers used for Debug Agent, this LogStreamResponse
     * class shall be removed, and the Rest::CustomResponse shall be refactored to take the
     * doBody method as lambda and become final.
     */
    class LogStreamResponse: public CustomResponse
    {
    public:
        LogStreamResponse(const std::string &contentType,
            std::unique_ptr<System::LogStreamResource> logStreamResource):
            CustomResponse(contentType),
            mLogStreamResource(std::move(logStreamResource))
        {
        }

        void doBodyResponse(std::ostream &out) override
        {
            try
            {
                mLogStreamResource->doLogStream(out);
            }
            catch (System::Exception &e)
            {
                throw Response::HttpAbort(std::string("cAVS Log stream error: ") + e.what());
            }
        }

    private:
        std::unique_ptr<System::LogStreamResource> mLogStreamResource;
    };

    /** Acquiring the log stream resource */
    auto &&resource = mSystem.tryToAcquireLogStreamResource();
    if (resource == nullptr) {
        throw Response::HttpError(Response::ErrorStatus::Locked,
            "Logging stream resource is already used.");
    }

    return std::make_unique<LogStreamResponse>(ContentTypeIfdkFile, std::move(resource));
    /**
     * @remarks Using C++14 lambda generalized capture, the return would be changed for:
     * @code
     * return std::make_unique<CustomResponse>(
     *   ContentTypeIfdkFile,
     *   [logResource = std::move(resource)](std::ostream out)
     *       {logResource->doLogStream(out);});
     */
}

}
}
