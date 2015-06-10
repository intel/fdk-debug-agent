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

/** Http request handler dedicated to REST
 * This handlers uses the dispatcher to resolve the resource, and then forwards the
 * request to this resource.*/
class RestResourceRequestHandler : public HTTPRequestHandler
{
public:
    RestResourceRequestHandler(std::shared_ptr<const Dispatcher> dispatcher) :
        mDispatcher(dispatcher) {}

    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp);

    /** Returns a failure to the client, using the supplied status code and error
     * message */
    void fail(HTTPResponse::HTTPStatus status, const std::string message,
        HTTPServerResponse &resp);

    static rest::Request::Verb translateVerb(const std::string &verbLiteral);

private:
    class UnknownVerbException : public std::logic_error
    {
    public:
        UnknownVerbException(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    std::shared_ptr<const Dispatcher> mDispatcher;
};

/* Http request handler factory required by Poco */
class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    RequestHandlerFactory(std::shared_ptr<const Dispatcher> dispatcher) :
        mDispatcher(dispatcher) {}

    virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &req);

private:
    std::shared_ptr<const Dispatcher> mDispatcher;
};

}
}