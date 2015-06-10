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
#include "cAVS/Resources.hpp"
#include <Poco/NumberParser.h>
#include <Poco/StringTokenizer.h>
#include <string>
#include <chrono>
#include <thread>

using namespace debug_agent::rest;

namespace debug_agent
{
namespace cavs
{

static const std::string ContentType("text/html");

void LogStreamResource::handleGet(const Request &request, Response &response)
{
    std::ostream &out = response.send(ContentType);
    /**
     * @todo replace this "demo" code by real implementation.
     */
    out << "<p>Not yet implemented</p>";
}

void LogParametersResource::handleGet(const Request &request, Response &response)
{
    std::ostream &out = response.send(ContentType);
    const Logger::Parameters logParameters = mSystem.getLogParameters();

    /**
     * @fixme This output is temporary. Final implementation will be done in a subsequent patch
     */
    out << "<p>"
        << logParameters.mIsStarted << ";"
        << Logger::toString(logParameters.mLevel) << ";"
        << Logger::toString(logParameters.mOutput)
        << "</p>";
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
    catch (Logger::Exception &e) {
        throw RequestException(ErrorStatus::BadRequest, std::string("Fail to apply: ") + e.what());
    }
    std::ostream &out = response.send(ContentType);
    out << "<p>Done</p>";
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
