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
#include "Util/AssertAlways.hpp"
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
        Response::sendHttpError(
            HTTPResponse::HTTP_NOT_FOUND, "Resource not found: " + req.getURI(), resp);
        return;
    }

    Request::Verb verb;
    try {
        verb = translateVerb(req.getMethod());
    }
    catch (UnknownVerbException &e)
    {
        Response::sendHttpError(
            HTTPResponse::HTTPStatus::HTTP_METHOD_NOT_ALLOWED, e.what(), resp);
        return;
    }

    /* Forwarding the request to the resource, that will handle it. */
    Request request(verb, req.stream(), *mIdentifiers);

    Resource::ResponsePtr response;

    try
    {
        try
        {
            response = mResource->handleRequest(request);
            if (response == nullptr) {

                throw Response::HttpError(Response::ErrorStatus::InternalError, "Response is null");
            }
        }
        catch (Response::HttpError &e)
        {
            /* Design is made to avoid this situation: always assert since HTTP header must have
             * not been sent already */
            ASSERT_ALWAYS(!resp.sent());

            HTTPResponse::HTTPStatus pocoStatus =
                static_cast<HTTPResponse::HTTPStatus>(e.getStatus());

            // HTTP error has to be sent to the client and we are done
            Response::sendHttpError(pocoStatus, e.what(), resp);
            return;
        }

        // Send HTTP response header
        response->sendHttpHeader(resp);

        try
        {
            // Send HTTP response body
            response->sendHttpBody();
        }
        catch (Response::HttpAbort &e)
        {
            /* Design is made to avoid this situation: always assert since HTTP header must have
             * been sent already */
            ASSERT_ALWAYS(resp.sent());

            // we cannot do anything else that log the issue and abandon the client
            /** @todo Use logging instead */
            std::cout << "Abort HTTP response on " << req.getURI() << ": "
                << e.what() << std::endl;
        }
    }
    catch (std::exception &e)
    {
        std::stringstream msg("Internal error: unexpected exception ");
        msg << typeid(e).name() << ": " << e.what();

        /** @todo Use logging instead */
        std::cout << msg.str();
        if (!resp.sent()) {
            Response::sendHttpError(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, msg.str(), resp);
        }
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

