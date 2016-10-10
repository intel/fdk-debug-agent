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
#include "catch.hpp"

using namespace debug_agent::rest;
using namespace debug_agent::test_common;

/* This resource returns the request verb  as response to the request client */
class VerbEchoResource : public Resource
{
public:
    virtual std::unique_ptr<Response> handleGet(const Request &) override
    {
        return std::make_unique<Response>("text/plain", "GET");
    }
    virtual std::unique_ptr<Response> handlePut(const Request &) override
    {
        return std::make_unique<Response>("text/plain", "PUT");
    }
    virtual std::unique_ptr<Response> handlePost(const Request &) override
    {
        return std::make_unique<Response>("text/plain", "POST");
    }
    virtual std::unique_ptr<Response> handleDelete(const Request &) override
    {
        return std::make_unique<Response>("text/plain", "DELETE");
    }
};

/**
 * This test checks that a default resource will reject all HTTP verbs
 */
TEST_CASE("Request test on default Resource rejecting all verbs", "[Server]")
{
    /* Initializing the dispatcher and the client */
    std::unique_ptr<Dispatcher> dispatcher = std::make_unique<Dispatcher>();
    HttpClientSimulator client("localhost");

    /* Each request will fail with Verb not allowed */
    std::string expectedResponseContent = "Verb not allowed";

    /* Adding resource */
    static const std::string testUri("/test/noverb");
    dispatcher->addResource(testUri, std::make_shared<Resource>());

    /* Starting the server */
    Server server(std::move(dispatcher), HttpClientSimulator::DefaultPort);

    SECTION ("GET") {

        /* Performing the http request */
        CHECK_NOTHROW(client.request(testUri,                                     // uri
                                     HttpClientSimulator::Verb::Get,              // verb
                                     "You will fail!",                            // request content
                                     HttpClientSimulator::Status::VerbNotAllowed, // expected status
                                     "text/plain", // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }

    SECTION ("POST") {

        /* Performing the http request */
        CHECK_NOTHROW(client.request(testUri,                                     // uri
                                     HttpClientSimulator::Verb::Post,             // verb
                                     "You will fail!",                            // request content
                                     HttpClientSimulator::Status::VerbNotAllowed, // expected status
                                     "text/plain", // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }

    SECTION ("PUT") {

        /* Performing the http request */
        CHECK_NOTHROW(client.request(testUri,                                     // uri
                                     HttpClientSimulator::Verb::Put,              // verb
                                     "You will fail!",                            // request content
                                     HttpClientSimulator::Status::VerbNotAllowed, // expected status
                                     "text/plain", // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }

    SECTION ("DELETE") {

        /* Performing the http request */
        CHECK_NOTHROW(client.request(testUri,                                     // uri
                                     HttpClientSimulator::Verb::Delete,           // verb
                                     "You will fail!",                            // request content
                                     HttpClientSimulator::Status::VerbNotAllowed, // expected status
                                     "text/plain", // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }
}

/**
 * This test checks that the correct verb default handler is called by the default handler
 */
TEST_CASE("Default verb handler", "[Server]")
{
    /* Initializing the dispatcher and the client */
    std::unique_ptr<Dispatcher> dispatcher = std::make_unique<Dispatcher>();
    HttpClientSimulator client("localhost");

    /* Adding resource */
    static const std::string testUri("/test/verb");
    dispatcher->addResource(testUri, std::make_shared<VerbEchoResource>());

    /* Starting the server */
    Server server(std::move(dispatcher), HttpClientSimulator::DefaultPort);

    SECTION ("GET") {

        std::string expectedResponseContent = "GET";
        /* Performing the http request */
        CHECK_NOTHROW(client.request(testUri,                         // uri
                                     HttpClientSimulator::Verb::Get,  // verb
                                     "",                              // request content
                                     HttpClientSimulator::Status::Ok, // expected status
                                     "text/plain",                    // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }

    SECTION ("PUT") {

        std::string expectedResponseContent = "PUT";
        /* Performing the http request */
        CHECK_NOTHROW(client.request(testUri,                         // uri
                                     HttpClientSimulator::Verb::Put,  // verb
                                     "",                              // request content
                                     HttpClientSimulator::Status::Ok, // expected status
                                     "text/plain",                    // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }

    SECTION ("POST") {

        std::string expectedResponseContent = "POST";
        /* Performing the http request */
        CHECK_NOTHROW(client.request(testUri,                         // uri
                                     HttpClientSimulator::Verb::Post, // verb
                                     "",                              // request content
                                     HttpClientSimulator::Status::Ok, // expected status
                                     "text/plain",                    // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }

    SECTION ("DELETE") {

        std::string expectedResponseContent = "DELETE";
        /* Performing the http request */
        CHECK_NOTHROW(client.request(testUri,                           // uri
                                     HttpClientSimulator::Verb::Delete, // verb
                                     "",                                // request content
                                     HttpClientSimulator::Status::Ok,   // expected status
                                     "text/plain",                      // expected content type
                                     HttpClientSimulator::StringContent(
                                         expectedResponseContent)) // expected response content
                      );
    }
}
