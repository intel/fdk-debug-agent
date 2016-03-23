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
#include "Util/StringHelper.hpp"
#include "Util/FileHelper.hpp"
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/NetException.h>
#include <Poco/StreamCopier.h>
#include <cassert>
#include <algorithm>
#include <fstream>

using namespace Poco::Net;

namespace debug_agent
{
namespace test_common
{

/* Translate the Poco status into the HttpClientSimulator::Status enum*/
static HttpClientSimulator::Status translateStatus(HTTPResponse::HTTPStatus status)
{
    switch (status) {
    case HTTPResponse::HTTPStatus::HTTP_OK:
        return HttpClientSimulator::Status::Ok;
    case HTTPResponse::HTTPStatus::HTTP_METHOD_NOT_ALLOWED:
        return HttpClientSimulator::Status::VerbNotAllowed;
    case HTTPResponse::HTTPStatus::HTTP_NOT_FOUND:
        return HttpClientSimulator::Status::NotFound;
    case static_cast<HTTPResponse::HTTPStatus>(423): /* This code is not defined by Poco */
        return HttpClientSimulator::Status::Locked;
    case HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR:
        return HttpClientSimulator::Status::InternalError;
    default:
        // There are around 40 different code in Poco, but only a few are expected
        throw HttpClientSimulator::RequestFailureException(std::string("Invalid http status"));
    }
}

/* Translate the HttpClientSimulator::Verb enum into Poco verb strings*/
const std::string &HttpClientSimulator::toString(Verb verb)
{
    switch (verb) {
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
    switch (s) {
    case Status::Ok:
        return "Ok";
    case Status::NotFound:
        return "NotFound";
    case Status::VerbNotAllowed:
        return "VerbNotAllowed";
    case Status::Locked:
        return "Locked";
    case Status::InternalError:
        return "InternalError";
    }
    throw RequestFailureException(std::string("Invalid http status"));
}

/* String content */

std::string HttpClientSimulator::StringContent::getSubStringSafe(const std::string &str,
                                                                 std::size_t index,
                                                                 std::size_t length)
{
    /* Changing the substring length if it exceeds the input string size */
    std::size_t safeLength;
    if (index + length <= str.length()) {
        safeLength = length;
    } else {
        safeLength = str.length() - index;
    }

    return str.substr(index, safeLength);
}

std::size_t HttpClientSimulator::StringContent::getStringDiffOffset(const std::string &str1,
                                                                    const std::string &str2)
{
    std::size_t minSize = std::min(str1.length(), str2.length());

    for (std::size_t i = 0; i < minSize; i++) {
        if (str1[i] != str2[i])
            return i;
    }
    return minSize;
}

void HttpClientSimulator::StringContent::checkExpected(const util::Buffer &content) const
{
    std::string stringContent(content.begin(), content.end());

    if (stringContent != mExpectedContent) {
        /* The substring that contains difference will not exceed 15 chars */
        static const std::size_t diffLength = 15;

        /* Getting the index of the first different char */
        std::size_t diffIndex = getStringDiffOffset(stringContent, mExpectedContent);

        /* "Got" content substring that contains the difference */
        std::string substringContent = getSubStringSafe(stringContent, diffIndex, diffLength);

        /* "Expected" content substring that contains the difference */
        std::string substringExpectedContent =
            getSubStringSafe(mExpectedContent, diffIndex, diffLength);

        throw RequestFailureException(
            "Wrong response content, got:\n" + stringContent + "\nexpected: '" + mExpectedContent +
            "' at index " + std::to_string(diffIndex) + ": got substring: '" + substringContent +
            "' expected substring: '" + substringExpectedContent + "'");
    }
}

/* File content */
void HttpClientSimulator::FileContent::checkExpected(const util::Buffer &gotContent) const
{
    try {
        auto expectedContent = util::file_helper::readAsBytes(mReferenceFile);

        if (gotContent != expectedContent) {
            std::string gotFile(mReferenceFile + "_got");
            util::file_helper::writeFromBytes(gotFile, gotContent);

            throw RequestFailureException("Wrong response content. See diff between:\n"
                                          "ref: " +
                                          mReferenceFile + "\ngot: " + gotFile);
        }
    } catch (util::file_helper::Exception &e) {
        throw RequestFailureException("File I/O error: " + std::string(e.what()));
    }
}

/* HttpClientSimulator */

void HttpClientSimulator::request(const std::string &uri, Verb verb,
                                  const std::string &requestContent, Status expectedStatus,
                                  const std::string &expectedContentType,
                                  const Content &expectedResponseContent)
{
    HTTPClientSession session(mServer, mPort);

    HTTPRequest request(toString(verb), uri);
    HTTPResponse response;
    util::Buffer responseContent;

    try {
        request.setChunkedTransferEncoding(true);
        request.setKeepAlive(true);

        /* Sending the request header */
        std::ostream &requestStream = session.sendRequest(request);

        /* Sending the request content */
        requestStream << requestContent;

        /* Receiving the response header */
        std::istream &responseStream = session.receiveResponse(response);

        /* Receiving the response content */

        responseContent = util::Buffer(std::istreambuf_iterator<char>(responseStream),
                                       std::istreambuf_iterator<char>());
    } catch (NetException &e) {
        throw NetworkException(std::string("Network error: ") + e.what());
    }

    /* Checking status */
    Status status = translateStatus(response.getStatus());
    if (status != expectedStatus) {
        throw RequestFailureException("Wrong status: '" + toString(status) + "' instead of '" +
                                      toString(expectedStatus) + "'");
    }
    /* Checking response content type */
    if (response.getContentType() != expectedContentType) {
        throw RequestFailureException("Wrong content-type: '" + response.getContentType() +
                                      "' instead of '" + expectedContentType + "'");
    }
    /* Checking response content*/
    expectedResponseContent.checkExpected(responseContent);
}
}
}
