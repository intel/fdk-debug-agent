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

#include "Rest/Dispatcher.hpp"
#include "catch.hpp"
#include <memory>

using namespace debug_agent::rest;

/* Creating a dummy resource that does nothing. */
class DummyResource : public Resource
{
public:
    virtual std::unique_ptr<Response> handleRequest(const Request &request)
    {
        (void)request;
        return nullptr;
    }
};

/* This method adds a resource into the dispatcher and checks that the URI is invalid. */
void checkAddResourceWithWrongURI(const std::string uri)
{
    Dispatcher dispatcher;
    std::shared_ptr<Resource> resource = std::make_shared<DummyResource>();
    REQUIRE_THROWS(dispatcher.addResource(uri, resource));
}

/* This method adds a resource into the dispatcher and checks that the URI is valid. */
void checkAddResourceWithValidURI(Dispatcher &dispatcher, const std::string uri)
{
    std::shared_ptr<Resource> resource = std::make_shared<DummyResource>();
    REQUIRE_NOTHROW(dispatcher.addResource(uri, resource));
}

/* This method tries to find a resource using the supplied URI, and checks that no resource is
 * found.*/
void checkFindResourceFailure(Dispatcher &dispatcher, const std::string &uri)
{
    Dispatcher::Identifiers identifiers;
    std::shared_ptr<Resource> resource = dispatcher.resolveResource(uri, identifiers);
    REQUIRE(resource == nullptr);
}

/* This method tries to find a resource using the supplied URI, and checks that:
 * - the resource is found
 * - the obtained identifiers are the expected ones.
 */
void checkFindResourceSuccess(
    Dispatcher &dispatcher, const std::string &uri,
    const Dispatcher::Identifiers &expectedIdentifiers = Dispatcher::Identifiers())
{
    Dispatcher::Identifiers identifiers;
    std::shared_ptr<Resource> resource = dispatcher.resolveResource(uri, identifiers);
    REQUIRE(resource != nullptr);
    REQUIRE(expectedIdentifiers == identifiers);
}

/* Checking that these URI are invalid for inserting a resource into the dispatcher */
TEST_CASE("Wrong URIs", "[Dispatcher]")
{
    checkAddResourceWithWrongURI("");
    checkAddResourceWithWrongURI("t");
    checkAddResourceWithWrongURI("t/");
    checkAddResourceWithWrongURI("/t/");                /* trailing slash is forbidden*/
    checkAddResourceWithWrongURI("/c//t///a");          /* multiple slash is forbidden */
    checkAddResourceWithWrongURI("/a#b/v~d");           /* unsupported characters*/
    checkAddResourceWithWrongURI("/a/${id eer1}");      /* wrong identifier name*/
    checkAddResourceWithWrongURI("/a/${id");            /* incomplete identifier*/
    checkAddResourceWithWrongURI("/a/ty${my-id}/c");    /* identifier name is not between slashes */
    checkAddResourceWithWrongURI("/a/${id1}/b/${id1}"); /* two identifiers with the same name*/
}

/* Checking valid URI for resource insertion, and trying to find  */
TEST_CASE("Valid URIs", "[Dispatcher]")
{
    Dispatcher dispatcher;

    /* Trying the root URL '/' */
    SECTION ("root") {
        /* Adding the resource */
        checkAddResourceWithValidURI(dispatcher, "/");

        /* Finding the resource with the same url*/
        checkFindResourceSuccess(dispatcher, "/");

        /* Should fail */
        checkFindResourceFailure(dispatcher, "//");
    }

    SECTION ("simple path with special characters") {
        /* Adding the resource */
        checkAddResourceWithValidURI(dispatcher, "/b/c-d/o_p");

        /* Finding the resource with the same url*/
        checkFindResourceSuccess(dispatcher, "/b/c-d/o_p");

        /* Trying a shorter or a longer URI: should fail */
        checkFindResourceFailure(dispatcher, "/b/c-d/o_p/a");
        checkFindResourceFailure(dispatcher, "/b/c-d");
    }

    SECTION ("Path with one identifier") {
        /* Adding the resource */
        checkAddResourceWithValidURI(dispatcher, "/ab/${id1}");

        /* wrong identifier value name (space are not allowed) */
        checkFindResourceFailure(dispatcher, "/ab/to to");

        /* Checking that the identifier has been fetched : id1=toto */
        Dispatcher::Identifiers identifiers;
        identifiers["id1"] = "toto";
        checkFindResourceSuccess(dispatcher, "/ab/toto", identifiers);
    }

    SECTION ("Path with three identifiers") {

        /* Adding the resource */
        checkAddResourceWithValidURI(dispatcher, "/ab/${id1}/cb/${id-p2}/${id3}");

        /* Checking that the identifiers has been fetched */
        Dispatcher::Identifiers identifiers;
        identifiers["id1"] = "toto";
        identifiers["id-p2"] = "titi8";
        identifiers["id3"] = "uu";
        checkFindResourceSuccess(dispatcher, "/ab/toto/cb/titi8/uu", identifiers);
    }

    SECTION ("simple path with a point") {
        /* Adding the resource */
        checkAddResourceWithValidURI(dispatcher, "/b/c.d/e.f");

        /* Finding the resource with the same url*/
        checkFindResourceSuccess(dispatcher, "/b/c.d/e.f");

        /* Trying a shorter or a longer URI: should fail */
        checkFindResourceFailure(dispatcher, "/b/c.d/e.f/a");
        checkFindResourceFailure(dispatcher, "/b/c.d");
    }
}
