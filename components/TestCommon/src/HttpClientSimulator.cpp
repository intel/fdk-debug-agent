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
#include <algorithm>

/** Unfortunately <Poco/Net/HTTPRequest.h> defines the "min" macro, which makes fail the
 * std::min function
 * So undefining it.
 */
#ifdef min
#undef min
#endif



using namespace Poco::Net;

namespace debug_agent
{
namespace test_common
{

const std::string HttpClientSimulator::AnyContent("<any_content>");

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

std::string HttpClientSimulator::getSubStringSafe(const std::string &str, std::size_t index,
    std::size_t length)
{
    /* Changing the substring length if it exceeds the input string size */
    std::size_t safeLength;
    if (index + length <= str.length()) {
        safeLength = length;
    }
    else {
        safeLength = str.length() - index;
    }

    return str.substr(index, safeLength);
}

std::size_t HttpClientSimulator::getStringDiffOffset(const std::string &str1,
    const std::string &str2)
{
    std::size_t minSize = std::min(str1.length(), str2.length());

    for (std::size_t i = 0; i < minSize; i++)
    {
        if (str1[i] != str2[i])
            return i;
    }
    return minSize;
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
        request.setKeepAlive(true);

        /* Sending the request header */
        std::ostream& requestStream = session.sendRequest(request);

        /* Sending the request content */
        requestStream << requestContent;

        /* Receiving the response header */
        std::istream &responseStream = session.receiveResponse(response);

        /* Receiving the response content */
        Poco::StreamCopier::copyToString(responseStream, responseContent);
    }
    catch (NetException &e)
    {
        throw NetworkException(std::string("Network error: ") + e.what());
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
    if (expectedResponseContent != AnyContent  && responseContent != expectedResponseContent) {

        /* The substring that contains difference will not exceed 15 chars */
        static const std::size_t diffLength = 15;

        /* Getting the index of the first different char */
        std::size_t diffIndex = getStringDiffOffset(responseContent, expectedResponseContent);

        /* "Got" content substring that contains the difference */
        std::string substringContent = getSubStringSafe(responseContent, diffIndex, diffLength);

        /* "Expected" content substring that contains the difference */
        std::string substringExpectedContent = getSubStringSafe(expectedResponseContent, diffIndex,
            diffLength);

        throw RequestFailureException(
            "Wrong response content, got:\n" + responseContent +
            "\nexpected: '" + expectedResponseContent + "' at index " + std::to_string(diffIndex)
            + ": got substring: '" + substringContent + "' expected substring: '" +
            substringExpectedContent + "'");
    }
}

}
}

