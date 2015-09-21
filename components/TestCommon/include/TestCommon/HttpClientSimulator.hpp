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

#pragma once

#include <string>
#include <inttypes.h>
#include <exception>
#include <cassert>
#include <stdexcept>

namespace debug_agent
{
namespace test_common
{

/**
 * This class simulates an http client using the Poco library.
 */
class HttpClientSimulator final
{
public:
    static const uint32_t DefaultPort = 9096;
    static const std::string AnyContent;

    /* Http verb */
    enum class Verb
    {
        Post,
        Get,
        Put,
        Delete
    };

    static const std::string &toString(Verb v);

    /* Http status */
    enum class Status
    {
        Ok,
        NotFound,
        VerbNotAllowed,
        Locked,
        InternalError
    };

    static std::string toString(Status s);

    /* This exception is thrown when the server response is not the expected one. */
    class RequestFailureException : public std::logic_error
    {
    public:
        RequestFailureException(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    /* This specialized exception is thrown when a network error occurs */
    class NetworkException : public RequestFailureException
    {
    public:
        NetworkException(const std::string &msg) : RequestFailureException(msg.c_str()) {}
    };

    HttpClientSimulator(const std::string &server, uint32_t port = DefaultPort) :
        mServer(server), mPort(port) {}

    /* Perform an http request and checks the result. If the result is not the expected one,
     * the exception RequestFailureException is thrown
     *
     * @param[in] uri the URI of the request
     * @param[in] verb the http verb
     * @param[in] requestContent the content of the request
     * @param[in] expectedStatus the expected http status returned by the server
     * @param[in] expectedContentType the expected response content type returned by the server
     * @param[in] expectedResponseContent the expected response content returned by the server.
     *                                    Use the AnyContent constant to accept any content.
     */
    void request(
        const std::string &uri,
        Verb verb,
        const std::string &requestContent,
        Status expectedStatus,
        const std::string &expectedContentType,
        const std::string &expectedResponseContent);

private:
    HttpClientSimulator(const HttpClientSimulator&) = delete;
    HttpClientSimulator &operator=(const HttpClientSimulator &) = delete;

    /** Returns a substring. If the substring exceeds the string length (i.e.
     * index + length > str.length()) then the returned substring is truncated (i.e. returning
     * chars from [index..str.length()] )
     */
    static std::string getSubStringSafe(const std::string &str, std::size_t index,
        std::size_t length);

    /** Returns the index of the first different char */
    std::size_t getStringDiffOffset(const std::string &str1, const std::string &str2);

    const std::string mServer;
    const uint32_t mPort;
};

}
}


