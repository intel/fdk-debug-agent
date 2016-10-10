/*
 * Copyright (c) 2015, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

    SECTION ("Resource not found") {
        /* Starting the server */
        Server server(std::move(dispatcher), HttpClientSimulator::DefaultPort);

        /* Performing the http request */
        CHECK_NOTHROW(
            client.request("/unknown",                            // uri
                           HttpClientSimulator::Verb::Get,        // verb
                           "",                                    // request content
                           HttpClientSimulator::Status::NotFound, // expected status
                           "text/plain",                          // expected content type
                           HttpClientSimulator::StringContent(
                               "Resource not found: /unknown")) // expected response content
            );
    }

    SECTION ("Resource without identifier") {

        /* Adding resource */
        dispatcher->addResource("/test/test2", std::make_shared<EchoResource>("text/html"));

        /* Starting the server */
        Server server(std::move(dispatcher), HttpClientSimulator::DefaultPort);

        /* Setting the expected response content */
        std::string expectedResponseContent = "Verb: DELETE\n"
                                              "Identifiers:\n"
                                              "Request content: Hello world!";

        /* Performing the http request */
        CHECK_NOTHROW(client.request("/test/test2",                     // uri
                                     HttpClientSimulator::Verb::Delete, // verb
                                     "Hello world!",                    // request content
                                     HttpClientSimulator::Status::Ok,   // expected status
                                     "text/html",                       // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }

    SECTION ("Resource with 2 identifiers") {

        /* Adding resource */
        dispatcher->addResource("/test/${i1}/titi/${i2}/lulu",
                                std::make_shared<EchoResource>("text/html"));

        /* Starting the server */
        Server server(std::move(dispatcher), HttpClientSimulator::DefaultPort);

        /* Setting the expected response content */
        std::string expectedResponseContent = "Verb: PUT\n"
                                              "Identifiers: i1=val1 i2=val2\n"
                                              "Request content: Two identifiers";

        /* Performing the http request */
        CHECK_NOTHROW(client.request("/test/val1/titi/val2/lulu",     // uri
                                     HttpClientSimulator::Verb::Put,  // verb
                                     "Two identifiers",               // request content
                                     HttpClientSimulator::Status::Ok, // expected status
                                     "text/html",                     // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }
}
