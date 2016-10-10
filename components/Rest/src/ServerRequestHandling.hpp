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

/* This is a private header*/

#pragma once

#include "Rest/Server.hpp"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NetException.h"
#include <stdexcept>

using namespace Poco::Net;

namespace debug_agent
{
namespace rest
{

/**
 * Http request handler dedicated to REST which handles a request to a resource.
 */
class RestResourceRequestHandler : public HTTPRequestHandler
{
public:
    RestResourceRequestHandler(std::shared_ptr<Resource> resource,
                               std::unique_ptr<Dispatcher::Identifiers> identifiers)
        : mResource(resource), mIdentifiers(std::move(identifiers))
    {
    }

    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp);

private:
    class UnknownVerbException : public std::logic_error
    {
    public:
        UnknownVerbException(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    static rest::Request::Verb translateVerb(const std::string &verbLiteral);

    std::shared_ptr<Resource> mResource;
    std::unique_ptr<Dispatcher::Identifiers> mIdentifiers;
};

/* Http request handler factory required by Poco */
class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    RequestHandlerFactory(std::unique_ptr<const Dispatcher> dispatcher, bool isVerbose)
        : mDispatcher(std::move(dispatcher)), mVerbose(isVerbose)
    {
    }

    virtual HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &req);

private:
    std::unique_ptr<const Dispatcher> mDispatcher;
    bool mVerbose;
};
}
}
