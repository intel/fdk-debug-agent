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
#include "Core/ModelConverter.hpp"
#include "Util/Uuid.hpp"
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

void ModuleEntryResource::handleGet(const Request &request, Response &response)
{
    /* Retrieving module entries, doesn't throw exception */
    const std::vector<ModuleEntry> &entries = mSystem.getModuleEntries();

    std::ostream &out = response.send(ContentTypeHtml);
    out << "<p>Module type count: " << entries.size() << "</p>";

    /* Writing the result as an html table */
    out << "<table border='1'><tr><td>name</td><td>uuid</td><td>module id</td></tr>";

    std::size_t moduleId = 0;
    for (auto &entry : entries)
    {
        out << "<tr>";

        /* Module Name */
        std::string name(
            util::StringHelper::getStringFromFixedSizeArray(entry.name, sizeof(entry.name)));
        out << "<td>" << name << "</td>";

        /* Module uuid */
        out << "<td>";

        util::Uuid uuid;
        uuid.fromOtherUuidType(entry.uuid);
        out << uuid.toString();

        out << "</td>";

        /* The module id is the array index */
        out << "<td>" << moduleId << "</td>";
        out << "</tr>";

        moduleId++;
    }
    out << "</table>";
}

void SystemTypeResource::handleGet(const Request &request, Response &response)
{
    type::System system;

    try {
        ModelConverter::getSystemType(system);
    }
    catch (ModelConverter::Exception &e)
    {
        throw HttpError(debug_agent::rest::Resource::ErrorStatus::InternalError,
            "Cannot create system type data model: " + std::string(e.what()));
    }

    xml::TypeSerializer serializer;
    system.accept(serializer);
    std::string xml = serializer.getXml();

    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void SystemInstanceResource::handleGet(const Request &request, Response &response)
{
    instance::System system;

    try {
        ModelConverter::getSystemInstance(system);
    }
    catch (ModelConverter::Exception &e)
    {
        throw HttpError(debug_agent::rest::Resource::ErrorStatus::InternalError,
            "Cannot create system instance data model: " + std::string(e.what()));
    }

    xml::InstanceSerializer serializer;
    system.accept(serializer);
    std::string xml = serializer.getXml();

    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void SubsystemTypeResource::handleGet(const Request &request, Response &response)
{
    /* Creating meta model */
    type::Subsystem subsystem;

    try {
        ModelConverter::getSubsystemType(subsystem,
            mSystem.getFwConfig(), mSystem.getHwConfig(), mSystem.getModuleEntries());
    }
    catch (ModelConverter::Exception &e)
    {
        throw HttpError(debug_agent::rest::Resource::ErrorStatus::InternalError,
            "Cannot create subsystem type data model: " + std::string(e.what()));
    }

    xml::TypeSerializer serializer;
    subsystem.accept(serializer);

    std::string xml = serializer.getXml();

    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void SubsystemsInstancesListResource::handleGet(const Request &request, Response &response)
{
    std::ostream &out = response.send(ContentTypeXml);

    out << "<subsystem_collection>"
        "    <subsystem Type=\"cavs\" Id=\"0\">"
        "        <info_parameters>"
        "            <ParameterBlock Name=\"Free Pages\">"
        "                <ParameterBlock Name=\"0\">"
        "                    <EnumParameter Name=\"mem_type\">HP_MEM</EnumParameter>"
        "                    <IntegerParameter Name=\"pages\">12</IntegerParameter>"
        "                </ParameterBlock>"
        "                <ParameterBlock Name=\"1\">"
        "                    <EnumParameter Name=\"mem_type\">LP_MEM</EnumParameter>"
        "                    <IntegerParameter Name=\"pages\">13</IntegerParameter>"
        "                </ParameterBlock>"
        "            </ParameterBlock>"
        "        </info_parameters>"
        "        <parents>"
        "            <system Type=\"SKL\" Id=\"0\"/>"
        "        </parents>"
        "        <children>"
        "            <collection Name=\"pipes\">"
        "                <!-- all pipe instances -->"
        "                <instance Type=\"pipe\" Id=\"0\"/>"
        "                <instance Type=\"pipe\" Id=\"1\"/>"
        "            </collection>"
        "            <collection Name=\"cores\">"
        "                <!-- all core instances -->"
        "                <instance Type=\"core\" Id=\"0\"/>"
        "                <instance Type=\"core\" Id=\"1\"/>"
        "            </collection>"
        "            <service_collection Name=\"services\">"
        "                <service Type=\"fwlogs\" Id=\"0\"/>"
        "            </service_collection>"
        "            <component_collection Name=\"modules\">"
        "                <!-- all module instances -->"
        "                <component Type=\"module-aec(2)\" Id=\"0\"/>"
        "                <component Type=\"module-gain(4)\" Id=\"3\"/>"
        "                <component Type=\"module-copier(1)\" Id=\"2\"/>"
        "            </component_collection>"
        "        </children>"
        "        <!-- links -->"
        "        <links>"
        "            <link Id=\"0\">"
        "                <from Type=\"module-aec(2)\" Id=\"0\" OutputId=\"1\"/>"
        "                <to Type=\"module-gain(4)\" Id=\"3\" InputId=\"0\"/>"
        "            </link>"
        "            <link Id=\"1\">"
        "                <from Type=\"module-gain(4)\" Id=\"3\" OutputId=\"2\"/>"
        "                <to Type=\"module-copier(1)\" Id=\"2\" InputId=\"0\"/>"
        "            </link>"
        "        </links>"
        "    </subsystem>"
        "</subsystem_collection>";
}

void SubsystemInstanceResource::handleGet(const Request &request, Response &response)
{
    Topology topology;
    try
    {
        mSystem.getTopology(topology);
    }
    catch (System::Exception &e)
    {
        throw HttpError(debug_agent::rest::Resource::ErrorStatus::InternalError,
            "Cannot get topology from fw: " + std::string(e.what()));
    }

    instance::Subsystem subsystem;
    try
    {
        ModelConverter::getSubsystemInstance(subsystem,
            mSystem.getFwConfig(), mSystem.getHwConfig(), mSystem.getModuleEntries(),
            topology);
    }
    catch (ModelConverter::Exception &e)
    {
        throw HttpError(debug_agent::rest::Resource::ErrorStatus::InternalError,
            "Cannot create subsystem instance data model: " + std::string(e.what()));
    }

    xml::InstanceSerializer serializer;
    subsystem.accept(serializer);

    std::string xml = serializer.getXml();

    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
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
    out << "<service Direction=\"Outgoing\" Type=\"fwlogs\" Id=\"0\">"
        "    <parents/>"
        "    <control_parameters>"
        "        <BooleanParameter Name=\"Started\">" <<
                    logParameters.mIsStarted << "</BooleanParameter>"
        "        <ParameterBlock Name=\"Buffering\">"
        "            <IntegerParameter Name=\"Size\">100</IntegerParameter>"
        "            <BooleanParameter Name=\"Circular\">0</BooleanParameter>"
        "        </ParameterBlock>"
        "        <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>"
        "        <EnumParameter Name=\"Verbosity\">" <<
                    Logger::toString(logParameters.mLevel) << "</EnumParameter>"
        "        <BooleanParameter Name=\"ViaPTI\">" <<
                    (logParameters.mOutput == Logger::Output::Pti ? 1 : 0) << "</BooleanParameter>"
        "    </control_parameters>"
        "</service>";
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
    out << "<control_parameters>"
        "    <BooleanParameter Name=\"Started\">" <<
        logParameters.mIsStarted << "</BooleanParameter>"
        "    <ParameterBlock Name=\"Buffering\">"
        "        <IntegerParameter Name=\"Size\">0</IntegerParameter>"
        "        <BooleanParameter Name=\"Circular\">0</BooleanParameter>"
        "    </ParameterBlock>"
        "    <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>"
        "    <EnumParameter Name=\"Verbosity\">" <<
        Logger::toString(logParameters.mLevel) << "</EnumParameter>"
        "    <BooleanParameter Name=\"ViaPTI\">" <<
        (logParameters.mOutput == Logger::Output::Pti ? 1 : 0) << "</BooleanParameter>"
        "</control_parameters>";
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
    out << "<service_type Name=\"fwlogs\">"
        "    <control_parameters>"
        "        <!-- service generic -->"
        "        <BooleanParameter Name=\"Started\"/>"
        "        <ParameterBlock Name=\"Buffering\">"
        "            <IntegerParameter Name=\"Size\" Size=\"16\" Unit=\"MegaBytes\"/>"
        "            <BooleanParameter Name=\"Circular\"/>"
        "        </ParameterBlock>"
        "        <BooleanParameter Name=\"PersistsState\"/>"
        "        <!-- service specific -->"
        "        <EnumParameter Size=\"8\" Name=\"Verbosity\">"
        "            <ValuePair Numerical=\"2\" Literal=\"Critical\"/>"
        "            <ValuePair Numerical=\"3\" Literal=\"High\"/>"
        "            <ValuePair Numerical=\"4\" Literal=\"Medium\"/>"
        "            <ValuePair Numerical=\"5\" Literal=\"Low\"/>"
        "            <ValuePair Numerical=\"6\" Literal=\"Verbose\"/>"
        "        </EnumParameter>"
        "        <BooleanParameter Name=\"ViaPTI\" " <<
                                  "Description=\"Set to 1 if PTI interface is to be used\"/>"
        "    </control_parameters>"
        "</service_type>";
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
