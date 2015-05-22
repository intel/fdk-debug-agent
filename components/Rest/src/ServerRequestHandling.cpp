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

#include "ServerRequestHandling.hpp"

using namespace Poco::Net;

namespace debug_agent
{
namespace rest
{

void RestResourceRequestHandler::handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
{
    Request::Verb verb;
    try {
        verb = translateVerb(req.getMethod());
    }
    catch (UnknownVerbException &e)
    {
        fail(HTTPResponse::HTTPStatus::HTTP_METHOD_NOT_ALLOWED, e.what(), resp);
        return;
    }

    /* Resolving the resource */
    Dispatcher::Identifiers identifiers;
    std::shared_ptr<Resource> resource =
        mDispatcher->resolveResource(req.getURI(), identifiers);

    if (resource == nullptr)
    {
        fail(HTTPResponse::HTTP_NOT_FOUND, "Resource not found: " + req.getURI(),
            resp);
        return;
    }

    /* Forwarding the request to the resource, that will handle it. */
    Request request(verb, req.stream(), identifiers);
    Response response(resp);

    try
    {
        resource->handleRequest(request, response);
    }
    catch (Resource::RequestException &e)
    {
        HTTPResponse::HTTPStatus pocoStatus =
            static_cast<HTTPResponse::HTTPStatus>(e.getStatus());

        fail(pocoStatus, e.what(), resp);
    }
    catch (ConnectionAbortedException &)
    {
        /* @todo Use logging instead */
        std::cout << "Connection aborted." << std::endl;
    }
}

void RestResourceRequestHandler::fail(HTTPResponse::HTTPStatus status, const std::string message,
    HTTPServerResponse &resp)
{
    resp.setStatus(status);
    resp.setContentType("text/plain");
    try
    {
        std::ostream &out = resp.send();
        out << message;
    }
    catch (ConnectionAbortedException &)
    {
        /* @todo Use logging instead */
        std::cout << "Connection aborted." << std::endl;
    }
}

Request::Verb RestResourceRequestHandler::translateVerb(const std::string &verbLiteral)
{
    if (verbLiteral == HTTPRequest::HTTP_GET)
        return Request::Verb::Get;
    if (verbLiteral == HTTPRequest::HTTP_POST)
        return Request::Verb::Post;
    if (verbLiteral == HTTPRequest::HTTP_PUT)
        return Request::Verb::Put;
    if (verbLiteral == HTTPRequest::HTTP_DELETE)
        return Request::Verb::Delete;
    throw UnknownVerbException("Unknown verb: " + verbLiteral);
}

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest &req)
{
    return new RestResourceRequestHandler(mDispatcher);
}

}
}

