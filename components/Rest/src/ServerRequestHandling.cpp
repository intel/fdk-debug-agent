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
#include "Rest/HttpMessageProperties.hpp"
#include <Poco/Exception.h>
#include <sstream>

using namespace Poco;
using namespace Poco::Net;

namespace debug_agent
{
namespace rest
{

void RestResourceRequestHandler::handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
{
    if (mResource == nullptr)
    {
        sendHttpError(HTTPResponse::HTTP_NOT_FOUND, "Resource not found: " + req.getURI(), resp);
        return;
    }

    Request::Verb verb;
    try {
        verb = translateVerb(req.getMethod());
    }
    catch (UnknownVerbException &e)
    {
        sendHttpError(HTTPResponse::HTTPStatus::HTTP_METHOD_NOT_ALLOWED, e.what(), resp);
        return;
    }

    /* Forwarding the request to the resource, that will handle it. */
    Request request(verb, req.stream(), *mIdentifiers);
    Response response(resp);

    try
    {
        mResource->handleRequest(request, response);
    }
    catch (Resource::HttpError &e)
    {
        HTTPResponse::HTTPStatus pocoStatus =
            static_cast<HTTPResponse::HTTPStatus>(e.getStatus());

        if (!resp.sent()) {
            // HTTP error has to be sent to the client
            sendHttpError(pocoStatus, e.what(), resp);
        }
        else {
            // we cannot do anything else that log the issue
            // This should not happen
            /** @todo Use logging instead */
            std::cout << "Internal error: "
                << "HttpError exception while response has already been sent: "
                << req.getURI() << ": " << e.what() << std::endl;
        }
    }
    catch (Resource::HttpAbort &e)
    {
        /** @todo Use logging instead */
        if (!resp.sent()) {
            // Should not happen
            /** @todo Use logging instead */
            std::cout << "Internal error: HttpAbort exception while response has not been sent: "
                << req.getURI() << ": " << e.what() << std::endl;
            // HTTP error has to be sent to the client
            sendHttpError(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, e.what(), resp);
        }
        else {
            // we cannot do anything else that log the issue and abandon the client
            /** @todo Use logging instead */
            std::cout << "Abort request on " << req.getURI() << ": " << e.what() << std::endl;
        }
    }
    catch (std::exception &e)
    {
        /** @todo Use logging instead */
        std::cout << "Abort request on " << req.getURI() << " due to unexpected exception: "
            << typeid(e).name() << ": " << e.what() << std::endl;
    }

    // Ensure we send at least an HTTP error response to the client
    if (!resp.sent()) {
        std::string msg("Internal error: no response sent out for: " + req.getURI());
        /** @todo Use logging instead */
        std::cout << msg;
        sendHttpError(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, msg, resp);
    }
}

void RestResourceRequestHandler::sendHttpError(HTTPResponse::HTTPStatus status,
                                               const std::string &message,
                                               HTTPServerResponse &resp)
{
    /* @todo Use logging instead */
    std::cout << "Request failure: " << message << std::endl;

    HttpMessageProperties::setCommonProperties(resp);
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
    /* Resolving the resource */
    std::unique_ptr<Dispatcher::Identifiers> identifiers =
        std::make_unique<Dispatcher::Identifiers>();
    std::shared_ptr<Resource> resource =
        mDispatcher->resolveResource(req.getURI(), *identifiers);

    /** @todo use log interface instead */
    if (mVerbose){
        std::cout << "RequestHandlerFactory::createRequestHandler "
                  << req.getMethod() << " " << req.getURI() << std::endl;
    }

    /* Poco forces us to use operator new here: the HttpServer will take the ownership of this new
     * RestResourceRequestHandler. */
    return new RestResourceRequestHandler(resource, std::move(identifiers));
}

}
}

