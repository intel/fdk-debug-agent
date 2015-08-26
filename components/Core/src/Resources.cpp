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
#include "Util/Uuid.hpp"
#include "IfdkObjects/Xml/TypeDeserializer.hpp"
#include "IfdkObjects/Xml/TypeSerializer.hpp"
#include "IfdkObjects/Xml/InstanceDeserializer.hpp"
#include "IfdkObjects/Xml/InstanceSerializer.hpp"
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

/** This method returns a std::string from a byte buffer
 *
 * @tparam ArrayElementType the array element type, its size must be one byte.
 *                          For instance: int8_t, uint8_t, char, unsigned char ...
 */
template<typename ArrayElementType>
static std::string getStringFromFixedSizeArray(ArrayElementType *buffer, std::size_t size)
{
    static_assert(sizeof(ArrayElementType) == 1, "Size of ArrayElementType must be one");

    std::stringstream stream;
    for (std::size_t i = 0; i < size && buffer[i] != 0; i++) {
        stream << static_cast<char>(buffer[i]);
    }
    return stream.str();
}

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
        std::string name(getStringFromFixedSizeArray(entry.name, sizeof(entry.name)));
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

/** @fixme To be removed when the switch to SystemTypeResource is complete
*
*   Temporary resource, to provide a Legacy System Type response. To be removed once the FDK Tool
*   is updated and does not need this message anymore.*/
void LegacySystemTypeResource::handleGet(const Request &request, Response &response)
{
    std::ostream &out = response.send(ContentTypeXml);

    out << "<system_type Name=\"SKL\">"
        "    <description>Skylake platform</description>"
        "    <subsystem_types>"
        "        <subsystem_type Name=\"cavs\"/>"
        "    </subsystem_types>"
        "</system_type>";
}

void SystemTypeResource::handleGet(const Request &request, Response &response)
{
    type::System system("bxtn");
    system.getDescription().setValue("Broxton platform");

    auto coll = new type::SubsystemRefCollection("subsystems");
    coll->add(type::SubsystemRef("cavs"));
    system.getChildren().add(coll);

    xml::TypeSerializer serializer;
    system.accept(serializer);
    std::string xml = serializer.getXml();

    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void SystemInstanceResource::handleGet(const Request &request, Response &response)
{
    instance::System system("bxtn", "0");

    auto coll = new instance::SubsystemRefCollection("subsystems");
    coll->add(instance::SubsystemRef("cavs", "0"));
    system.getChildren().add(coll);

    xml::InstanceSerializer serializer;
    system.accept(serializer);
    std::string xml = serializer.getXml();

    std::ostream &out = response.send(ContentTypeXml);
    out << xml;
}

void SubsystemTypeResource::addSystemCharacteristics(type::Characteristics &ch)
{
    // Add FW config
    const FwConfig &fwConfig = mSystem.getFwConfig();
    if (fwConfig.isFwVersionValid) {
        ch.add(type::Characteristic(
            "Firmware version",
            std::to_string(fwConfig.fwVersion.major) + "." +
            std::to_string(fwConfig.fwVersion.minor) + "." +
            std::to_string(fwConfig.fwVersion.hotfix) + "." +
            std::to_string(fwConfig.fwVersion.build)));
    }
    if (fwConfig.isMemoryReclaimedValid) {
        ch.add(type::Characteristic(
            "Memory reclaimed",
            std::to_string(fwConfig.memoryReclaimed)));
    }
    if (fwConfig.isSlowClockFreqHzValid) {
        ch.add(type::Characteristic(
            "Slow clock frequency (Hz)",
            std::to_string(fwConfig.slowClockFreqHz)));
    }
    if (fwConfig.isFastClockFreqHzValid) {
        ch.add(type::Characteristic(
            "Fast clock frequency (Hz)",
            std::to_string(fwConfig.fastClockFreqHz)));
    }
    if (fwConfig.dmaBufferConfig.size() > 0) {
        size_t i = 0;
        for (auto dmaBufferConfig : fwConfig.dmaBufferConfig) {
            ch.add(type::Characteristic(
                "DMA buffer config #" + std::to_string(i) + " min size (bytes)",
                std::to_string(dmaBufferConfig.min_size_bytes)));
            ch.add(type::Characteristic(
                "DMA buffer config #" + std::to_string(i) + " max size (bytes)",
                std::to_string(dmaBufferConfig.max_size_bytes)));
            ++i;
        }
    }
    if (fwConfig.isAlhSupportLevelValid) {
        ch.add(type::Characteristic(
            "Audio Hub Link support level",
            std::to_string(fwConfig.alhSupportLevel)));
    }
    if (fwConfig.isIpcDlMailboxBytesValid) {
        ch.add(type::Characteristic(
            "IPC down link (host to FW) mailbox size (bytes)",
            std::to_string(fwConfig.ipcDlMailboxBytes)));
    }
    if (fwConfig.isIpcUlMailboxBytesValid) {
        ch.add(type::Characteristic(
            "IPC up link (FW to host) mailbox size (bytes)",
            std::to_string(fwConfig.ipcUlMailboxBytes)));
    }
    if (fwConfig.isTraceLogBytesValid) {
        ch.add(type::Characteristic(
            "Size of trace log buffer per single core (bytes)",
            std::to_string(fwConfig.traceLogBytes)));
    }
    if (fwConfig.isMaxPplCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of pipelines instances",
            std::to_string(fwConfig.maxPplCount)));
    }
    if (fwConfig.isMaxAstateCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of A-state table entries",
            std::to_string(fwConfig.maxAstateCount)));
    }
    if (fwConfig.isMaxModulePinCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of input or output pins per module",
            std::to_string(fwConfig.maxModulePinCount)));
    }
    if (fwConfig.isModulesCountValid) {
        ch.add(type::Characteristic(
            "Current total number of module entries loaded into the DSP",
            std::to_string(fwConfig.modulesCount)));
    }
    if (fwConfig.isMaxModInstCountValid) {
        ch.add(type::Characteristic(
            "Maximum module instance count",
            std::to_string(fwConfig.maxModInstCount)));
    }
    if (fwConfig.isMaxLlTasksPerPriCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of LL tasks per priority",
            std::to_string(fwConfig.maxLlTasksPerPriCount)));
    }
    if (fwConfig.isLlPriCountValid) {
        ch.add(type::Characteristic(
            "Number of LL priorities",
            std::to_string(fwConfig.llPriCount)));
    }
    if (fwConfig.isMaxDpTasksCountValid) {
        ch.add(type::Characteristic(
            "Maximum number of DP tasks per core",
            std::to_string(fwConfig.maxDpTasksCount)));
    }

    // Add HW config
    const HwConfig &hwConfig = mSystem.getHwConfig();
    if (hwConfig.isCavsVersionValid) {
        ch.add(type::Characteristic(
            "cAVS Version",
            std::to_string(hwConfig.cavsVersion)));
    }
    if (hwConfig.isDspCoreCountValid) {
        ch.add(type::Characteristic(
            "Number of cores",
            std::to_string(hwConfig.dspCoreCount)));
    }
    if (hwConfig.isMemPageSizeValid) {
        ch.add(type::Characteristic(
            "Memory page size (bytes)",
            std::to_string(hwConfig.memPageSize)));
    }
    if (hwConfig.isTotalPhysicalMemoryPageValid) {
        ch.add(type::Characteristic(
            "Total number of physical pages",
            std::to_string(hwConfig.totalPhysicalMemoryPage)));
    }
    if (hwConfig.isI2sCapsValid) {
        ch.add(type::Characteristic(
            "I2S version",
            std::to_string(hwConfig.i2sCaps.version)));
        size_t i = 0;
        for (auto controllerBaseAddr : hwConfig.i2sCaps.controllerBaseAddr) {

            ch.add(type::Characteristic(
                "I2S controller #" + std::to_string(i++) + " base address",
                std::to_string(controllerBaseAddr)));
        }
    }
    if (hwConfig.gatewayCount) {
        ch.add(type::Characteristic(
            "Total number of DMA gateways",
            std::to_string(hwConfig.gatewayCount)));
    }
    if (hwConfig.isEbbCountValid) {
        ch.add(type::Characteristic(
            "Number of SRAM memory banks",
            std::to_string(hwConfig.ebbCount)));
    }
}

void SubsystemTypeResource::handleGet(const Request &request, Response &response)
{
    static const std::vector<std::string> staticTypeCollections = {
        "pipes", "cores", "tasks"
    };

    static const std::vector<std::string> staticTypes = {
        "pipe", "core", "task"
    };

    static const std::vector<std::string> staticServiceTypes = {
        "fwlogs"
    };

    static const std::vector<std::string> gateways = {
        "hda-host-out-gateway",
        "hda-host-in-gateway",
        "hda-link-out-gateway",
        "hda-link-in-gateway",
        "dmic-link-in-gateway"
    };

    /* Creating meta model */
    type::Subsystem subsystem("cavs");
    subsystem.getDescription().setValue("cAVS subsystem");

    /* Hardcoded characteristics (temporary) */
    type::Characteristics &ch = subsystem.getCharacteristics();
    addSystemCharacteristics(ch);

    /* Children and categories */
    type::Children &children = subsystem.getChildren();
    type::Categories &categories = subsystem.getCategories();

    /* Static types */
    assert(staticTypeCollections.size() == staticTypes.size());
    for (std::size_t i = 0; i < staticTypeCollections.size(); ++i) {
        auto coll = new type::TypeRefCollection(staticTypeCollections[i]);
        coll->add(type::TypeRef(staticTypes[i]));
        children.add(coll);

        categories.add(new type::TypeRef(staticTypes[i]));
    }

    /* Service */
    auto serviceColl = new type::ServiceRefCollection("services");
    for (auto &serviceName : staticServiceTypes) {
        serviceColl->add(type::ServiceRef(serviceName));

        categories.add(new type::ServiceRef(serviceName));
    }
    children.add(serviceColl);

    /* Gateways */
    auto gatewayColl = new type::ComponentRefCollection("gateways");
    for (auto &gatewayName : gateways) {
        gatewayColl->add(type::ComponentRef(gatewayName));

        categories.add(new type::ComponentRef(gatewayName));
    }
    children.add(gatewayColl);

    /* Modules*/
    auto compColl = new type::ComponentRefCollection("modules");

    const std::vector<ModuleEntry> &entries = mSystem.getModuleEntries();
    for (auto &module : entries) {
        std::string moduleName = getStringFromFixedSizeArray(module.name, sizeof(module.name));
        compColl->add(type::ComponentRef(moduleName));

        categories.add(new type::ComponentRef(moduleName));
    }
    children.add(compColl);

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
    std::ostream &out = response.send(ContentTypeXml);
    out << "<subsystem Type=\"cavs\" Id=\"0\">"
        "    <info_parameters>"
        "        <ParameterBlock Name=\"Free Pages\">"
        "            <ParameterBlock Name=\"0\">"
        "                <EnumParameter Name=\"mem_type\">HP_MEM</EnumParameter>"
        "                <IntegerParameter Name=\"pages\">12</IntegerParameter>"
        "            </ParameterBlock>"
        "            <ParameterBlock Name=\"1\">"
        "                <EnumParameter Name=\"mem_type\">LP_MEM</EnumParameter>"
        "                <IntegerParameter Name=\"pages\">13</IntegerParameter>"
        "            </ParameterBlock>"
        "        </ParameterBlock>"
        "    </info_parameters>"
        "    <parents>"
        "        <system Type=\"SKL\" Id=\"0\"/>"
        "    </parents>"
        "    <children>"
        "        <collection Name=\"pipes\">"
        "            <!-- all pipe instances -->"
        "            <instance Type=\"pipe\" Id=\"0\"/>"
        "            <instance Type=\"pipe\" Id=\"1\"/>"
        "        </collection>"
        "        <collection Name=\"cores\">"
        "            <!-- all core instances -->"
        "            <instance Type=\"core\" Id=\"0\"/>"
        "            <instance Type=\"core\" Id=\"1\"/>"
        "        </collection>"
        "        <service_collection Name=\"services\">"
        "            <service Type=\"fwlogs\" Id=\"0\"/>"
        "        </service_collection>"
        "        <component_collection Name=\"modules\">"
        "            <!-- all module instances -->"
        "            <component Type=\"module-aec(2)\" Id=\"0\"/>"
        "            <component Type=\"module-gain(4)\" Id=\"3\"/>"
        "            <component Type=\"module-copier(1)\" Id=\"2\"/>"
        "        </component_collection>"
        "    </children>"
        "    <!-- links -->"
        "    <links>"
        "        <link Id=\"0\">"
        "            <from Type=\"module-aec(2)\" Id=\"0\" OutputId=\"1\"/>"
        "            <to Type=\"module-gain(4)\" Id=\"3\" InputId=\"0\"/>"
        "        </link>"
        "        <link Id=\"1\">"
        "            <from Type=\"module-gain(4)\" Id=\"3\" OutputId=\"2\"/>"
        "            <to Type=\"module-copier(1)\" Id=\"2\" InputId=\"0\"/>"
        "        </link>"
        "    </links>"
        "</subsystem>";
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
