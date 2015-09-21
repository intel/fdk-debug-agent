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
#include "catch.hpp"

using namespace debug_agent::rest;
using namespace debug_agent::test_common;

/* This resource returns the request verb  as response to the request client */
class VerbEchoResource : public Resource
{
public:
    virtual void handleGet(const Request &request, Response &response) override
    {
        std::ostream &out = response.send("text/plain");
        out << "GET";
    }
    virtual void handlePut(const Request &request, Response &response) override
    {
        std::ostream &out = response.send("text/plain");
        out << "PUT";
    }
    virtual void handlePost(const Request &request, Response &response) override
    {
        std::ostream &out = response.send("text/plain");
        out << "POST";
    }
    virtual void handleDelete(const Request &request, Response &response) override
    {
        std::ostream &out = response.send("text/plain");
        out << "DELETE";
    }
};

/**
 * This test checks that a default resource will reject all HTTP verbs
 */
TEST_CASE("Request test on default Resource rejecting all verbs", "[Server]")
{
    /* Initializing the dispatcher and the client */
    std::shared_ptr<Dispatcher> dispatcher = std::make_shared<Dispatcher>();
    HttpClientSimulator client("localhost");

    /* Each request will fail with Verb not allowed */
    std::string expectedResponseContent = "Verb not allowed";

    /* Adding resource */
    static const std::string testUri("/test/noverb");
    dispatcher->addResource(testUri, std::make_shared<Resource>());

    /* Starting the server */
    Server server(dispatcher, HttpClientSimulator::DefaultPort);

    SECTION("GET") {

        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                testUri, // uri
                HttpClientSimulator::Verb::Get, // verb
                "You will fail!", // request content
                HttpClientSimulator::Status::VerbNotAllowed, // expected status
                "text/plain", // expected content type
                expectedResponseContent) // expected response content
            );
    }

    SECTION("POST") {

        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                testUri, // uri
                HttpClientSimulator::Verb::Post, // verb
                "You will fail!", // request content
                HttpClientSimulator::Status::VerbNotAllowed, // expected status
                "text/plain", // expected content type
                expectedResponseContent) // expected response content
            );
    }

    SECTION("PUT") {

        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                testUri, // uri
                HttpClientSimulator::Verb::Put, // verb
                "You will fail!", // request content
                HttpClientSimulator::Status::VerbNotAllowed, // expected status
                "text/plain", // expected content type
                expectedResponseContent) //expected response content
            );
    }

    SECTION("DELETE") {

        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                testUri, // uri
                HttpClientSimulator::Verb::Delete, // verb
                "You will fail!", // request content
                HttpClientSimulator::Status::VerbNotAllowed, // expected status
                "text/plain", // expected content type
                expectedResponseContent) // expected response content
            );
    }
}

/**
 * This test checks that the correct verb default handler is called by the default handler
 */
TEST_CASE("Default verb handler", "[Server]")
{
    /* Initializing the dispatcher and the client */
    std::shared_ptr<Dispatcher> dispatcher = std::make_shared<Dispatcher>();
    HttpClientSimulator client("localhost");

    /* Adding resource */
    static const std::string testUri("/test/verb");
    dispatcher->addResource(testUri, std::make_shared<VerbEchoResource>());

    /* Starting the server */
    Server server(dispatcher, HttpClientSimulator::DefaultPort);

    SECTION("GET") {

        std::string expectedResponseContent = "GET";
        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                testUri, // uri
                HttpClientSimulator::Verb::Get, // verb
                "", // request content
                HttpClientSimulator::Status::Ok, // expected status
                "text/plain", // expected content type
                expectedResponseContent) // expected response content
            );
    }

    SECTION("PUT") {

        std::string expectedResponseContent = "PUT";
        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                testUri, // uri
                HttpClientSimulator::Verb::Put, // verb
                "", // request content
                HttpClientSimulator::Status::Ok, // expected status
                "text/plain", // expected content type
                expectedResponseContent) // expected response content
            );
    }

    SECTION("POST") {

        std::string expectedResponseContent = "POST";
        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                testUri, // uri
                HttpClientSimulator::Verb::Post, // verb
                "", // request content
                HttpClientSimulator::Status::Ok, // expected status
                "text/plain", // expected content type
                expectedResponseContent) // expected response content
            );
    }

    SECTION("DELETE") {

        std::string expectedResponseContent = "DELETE";
        /* Performing the http request */
        CHECK_NOTHROW(
            client.request(
                testUri, // uri
                HttpClientSimulator::Verb::Delete, // verb
                "", // request content
                HttpClientSimulator::Status::Ok, // expected status
                "text/plain", // expected content type
                expectedResponseContent) // expected response content
            );
    }
}
