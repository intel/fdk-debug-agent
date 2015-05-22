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

#include "TestCommon/HttpClientSimulator.hpp"
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/StreamCopier.h>
#include <assert.h>

using namespace Poco::Net;

namespace debug_agent
{
namespace test_common
{

/* Translate the Poco status into the HttpClientSimulator::Status enum*/
static HttpClientSimulator::Status translateStatus(HTTPResponse::HTTPStatus status)
{
    switch (status)
    {
    case HTTPResponse::HTTPStatus::HTTP_OK:
        return HttpClientSimulator::Status::Ok;
    case HTTPResponse::HTTPStatus::HTTP_METHOD_NOT_ALLOWED:
        return HttpClientSimulator::Status::VerbNotAllowed;
    case HTTPResponse::HTTPStatus::HTTP_NOT_FOUND:
        return HttpClientSimulator::Status::NotFound;
    }
    throw HttpClientSimulator::RequestFailureException(std::string("Invalid http status"));
}

/* Translate the HttpClientSimulator::Verb enum into Poco verb strings*/
const std::string &HttpClientSimulator::toString(Verb verb)
{
    switch (verb)
    {
    case Verb::Post:
        return HTTPRequest::HTTP_POST;
    case Verb::Get:
        return HTTPRequest::HTTP_GET;
    case Verb::Put:
        return HTTPRequest::HTTP_PUT;
    case Verb::Delete:
        return HTTPRequest::HTTP_DELETE;
    }
    throw RequestFailureException(std::string("Invalid http verb"));
}

std::string HttpClientSimulator::toString(Status s)
{
    switch (s)
    {
    case Status::Ok:
        return "Ok";
    case Status::NotFound:
        return "NotFound";
    case Status::VerbNotAllowed:
        return "VerbNotAllowed";
    }
    throw RequestFailureException(std::string("Invalid http status"));
}

void HttpClientSimulator::request(
    const std::string &uri,
    Verb verb,
    const std::string &requestContent,
    Status expectedStatus,
    const std::string &expectedContentType,
    const std::string &expectedResponseContent)
{
    HTTPClientSession session(mServer, mPort);

    HTTPRequest request(toString(verb), uri);
    HTTPResponse response;
    std::string responseContent;

    try
    {
        request.setChunkedTransferEncoding(true);

        /* Sending the request header */
        std::ostream& requestStream = session.sendRequest(request);

        /* Sending the request content */
        requestStream << requestContent;

        /* Receiving the response header */
        std::istream &responseStream = session.receiveResponse(response);

        /* Receiving the response content */
        Poco::StreamCopier::copyToString(responseStream, responseContent);
    }
    catch (ConnectionAbortedException &e)
    {
        throw RequestFailureException(std::string("Connection aborted: ") + e.what());
    }

    /* Checking status */
    Status status = translateStatus(response.getStatus());
    if (status != expectedStatus) {
        throw RequestFailureException("Wrong status: '" + toString(status) +
            "' instead of '" + toString(expectedStatus) + "'");

    }
    /* Checking response content type */
    if (response.getContentType() != expectedContentType) {
        throw RequestFailureException("Wrong content-type: '" + response.getContentType() +
            "' instead of '" + expectedContentType + "'");

    }
    /* Checking response content*/
    if (responseContent != expectedResponseContent) {
        throw RequestFailureException(
            "Wrong response content, got:\n" + responseContent +
            "\nexpected: '" + expectedResponseContent + "'");
    }
}

}
}

