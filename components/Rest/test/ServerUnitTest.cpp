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

#include "Rest/Server.hpp"
#include "TestCommon/HttpClientSimulator.hpp"
#include "Poco/StreamCopier.h"
#include "catch.hpp"
#include <memory>

using namespace debug_agent::rest;
using namespace debug_agent::test_common;

/* This resource sends the request properties (verb, identifiers, content) back to the client  */
class EchoResource : public Resource
{
public:
    EchoResource(const std::string contentType) : mContentType(contentType) {}

    virtual std::unique_ptr<Response> handleRequest(const Request &request)
    {
        /* Writing request properties into the response stream */

        std::stringstream responseStream;
        responseStream << "Verb: " << Request::toString(request.getVerb()) << "\n"
            << "Identifiers:";

        std::string requestContent;
        Poco::StreamCopier::copyToString(request.getRequestStream(), requestContent);

        for (auto &param : request.getIdentifiers()) {
            responseStream << " " << param.first << "=" << param.second;
        }

        responseStream << "\nRequest content: " << requestContent;

        return std::make_unique<Response>(mContentType, responseStream.str());
    }

private:
    const std::string mContentType;
};

TEST_CASE("Request test", "[Server]")
{
    /* Initializing the dispatcher and the client */
    std::unique_ptr<Dispatcher> dispatcher = std::make_unique<Dispatcher>();
    HttpClientSimulator client("localhost");

    SECTION("Resource not found")
    {
        /* Starting the server */
        Server server(std::move(dispatcher), HttpClientSimulator::DefaultPort);

        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                "/unknown", // uri
                HttpClientSimulator::Verb::Get,  //verb
                "",  // request content
                HttpClientSimulator::Status::NotFound, //expected status
                "text/plain", //expected content type
                "Resource not found: /unknown") //expected response content
            );
    }

    SECTION("Resource without identifier") {

        /* Adding resource */
        dispatcher->addResource("/test/test2", std::make_shared<EchoResource>("text/html"));

        /* Starting the server */
        Server server(std::move(dispatcher), HttpClientSimulator::DefaultPort);

        /* Setting the expected response content */
        std::string expectedResponseContent =
            "Verb: DELETE\n"
            "Identifiers:\n"
            "Request content: Hello world!";

        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                "/test/test2", // uri
                HttpClientSimulator::Verb::Delete,  //verb
                "Hello world!",  // request content
                HttpClientSimulator::Status::Ok, //expected status
                "text/html", //expected content type
                expectedResponseContent) //expected response content
            );
    }

    SECTION("Resource with 2 identifiers") {

        /* Adding resource */
        dispatcher->addResource("/test/${i1}/titi/${i2}/lulu",
            std::make_shared<EchoResource>("text/html"));

        /* Starting the server */
        Server server(std::move(dispatcher), HttpClientSimulator::DefaultPort);

        /* Setting the expected response content */
        std::string expectedResponseContent =
            "Verb: PUT\n"
            "Identifiers: i1=val1 i2=val2\n"
            "Request content: Two identifiers";

        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                "/test/val1/titi/val2/lulu", // uri
                HttpClientSimulator::Verb::Put,  //verb
                "Two identifiers",  // request content
                HttpClientSimulator::Status::Ok, //expected status
                "text/html", //expected content type
                expectedResponseContent) //expected response content
            );
    }
}
