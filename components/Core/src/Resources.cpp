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
#include <Poco/NumberParser.h>
#include <Poco/StringTokenizer.h>
#include <string>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

using namespace debug_agent::rest;
using namespace debug_agent::cavs;

namespace debug_agent
{
namespace core
{

static const std::string ContentType("text/html");

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

void LogStreamResource::handleGet(const Request &request, Response &response)
{
    /** Acquiring the log stream resource */
    auto resource = std::move(mSystem.tryToAcquireLogStreamResource());
    if (resource == nullptr) {
        throw RequestException(ErrorStatus::Locked, "Logging stream resource is already used.");
    }

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
    static const std::string ContentType("application/vnd.ifdk-file");
    std::ostream &out = response.send(ContentType);

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

void LogParametersResource::handleGet(const Request &request, Response &response)
{
    Logger::Parameters logParameters;

    try
    {
        logParameters = mSystem.getLogParameters();
    }
    catch (System::Exception &e)
    {
        throw RequestException(ErrorStatus::BadRequest, "Cannot get log parameters : " +
            std::string(e.what()));
    }

    std::ostream &out = response.send(ContentType);

    /**
     * @fixme This output is temporary. Final implementation will be done in a subsequent patch
     */
    out << "<table border='1'><tr><th>Log Parameter</th><th>Value</th></tr>"
        << "<tr><td>State</td><td>" << logParameters.mIsStarted << "</td></tr>"
        << "<tr><td>Level</td><td>" << Logger::toString(logParameters.mLevel) << "</td></tr>"
        << "<tr><td>Output</td><td>" << Logger::toString(logParameters.mOutput)
        << "</td></tr></table>"
        << "<p>To change log parameters: "
        << "PUT [log status];[log level];[log output] at /cAVS/logging/parameters</p>";
}

void LogParametersResource::handlePut(const Request &request, Response &response)
{
    std::string parameters = request.getRequestContentAsString();
    Poco::StringTokenizer parametersList(parameters, delimiters, Poco::StringTokenizer::TOK_TRIM);

    /**
     * @fixme This request format is temporary. Final implementation will be done in a subsequent
     * patch
     */
    if (parametersList.count() != numberOfParameters) {
        throw RequestException(ErrorStatus::BadRequest, "Invalid parameters format");
    }

    Logger::Parameters logParameters;
    try {
        logParameters.mIsStarted =
            Poco::NumberParser::parseBool(parametersList[isStartedParameterIndex]);
        logParameters.mLevel =
            Logger::levelFromString(parametersList[levelParameterIndex]);
        logParameters.mOutput =
            Logger::outputFromString(parametersList[outputParameterIndex]);
    }
    catch (Logger::Exception &e) {
        throw RequestException(ErrorStatus::BadRequest, std::string("Invalid value: ") + e.what());
    }
    catch (Poco::SyntaxException &e) {
        throw RequestException(ErrorStatus::BadRequest,
            std::string("Invalid Start/Stop request: ") + e.what());
    }

    try {
        mSystem.setLogParameters(logParameters);
    }
    catch (System::Exception &e) {
        throw RequestException(ErrorStatus::BadRequest, std::string("Fail to apply: ") + e.what());
    }

    std::ostream &out = response.send(ContentType);
    out << "<p>Done</p>";
}

void ModuleEntryResource::handleGet(const Request &request, Response &response)
{
    /* Retrieving module entries, doesn't throw exception */
    const std::vector<dsp_fw::ModuleEntry> &entries = mSystem.getModuleEntries();

    std::ostream &out = response.send(ContentType);
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

/**
 * @fixme These constants are temporary, final implementation will not use them and they will be
 * removed.
 */
const std::string LogParametersResource::delimiters(";");
const std::size_t LogParametersResource::numberOfParameters = 3;
const std::size_t LogParametersResource::isStartedParameterIndex = 0;
const std::size_t LogParametersResource::levelParameterIndex = 1;
const std::size_t LogParametersResource::outputParameterIndex = 2;
}
}
