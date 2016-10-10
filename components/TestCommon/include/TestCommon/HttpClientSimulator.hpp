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

#pragma once

#include <Util/Buffer.hpp>
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
    class RequestFailureException : public std::runtime_error
    {
    public:
        RequestFailureException(const std::string &msg) : std::runtime_error(msg.c_str()) {}
    };

    /* This specialized exception is thrown when a network error occurs */
    class NetworkException : public RequestFailureException
    {
    public:
        NetworkException(const std::string &msg) : RequestFailureException(msg.c_str()) {}
    };

    HttpClientSimulator(const std::string &server, uint32_t port = DefaultPort)
        : mServer(server), mPort(port)
    {
    }

    /* Abstract expected http request content */
    class Content
    {
    public:
        virtual ~Content() {}
        /** Check that the content passed as parameter is the expected one */
        virtual void checkExpected(const util::Buffer &content) const = 0;
    };

    /* Accepts any content */
    class AnyContent : public Content
    {
    public:
        void checkExpected(const util::Buffer & /*content*/) const override {}
    };

    /* Checking the http request content from a string.
     *
     * If the check fails, an exception is thrown with a message that helps to locate the
     * diff within the content.
     */
    class StringContent : public Content
    {
    public:
        explicit StringContent(const std::string &expectedContent)
            : mExpectedContent(expectedContent)
        {
        }

        void checkExpected(const util::Buffer &content) const override;

    private:
        const std::string mExpectedContent;

        /** Returns a substring. If the substring exceeds the string length (i.e.
        * index + length > str.length()) then the returned substring is truncated (i.e. returning
        * chars from [index..str.length()] )
        */
        static std::string getSubStringSafe(const std::string &str, std::size_t index,
                                            std::size_t length);

        /** Returns the index of the first different char */
        static std::size_t getStringDiffOffset(const std::string &str1, const std::string &str2);
    };

    /* Checking the http request content from a file.
     *
     * If the check fails, a file with the name <reference file name> + "_got" is created,
     * that contains the "got" content. The user can make a diff between the two files.
     */
    class FileContent : public Content
    {
    public:
        explicit FileContent(const std::string &referenceFile) : mReferenceFile(referenceFile) {}

        void checkExpected(const util::Buffer &content) const override;

    private:
        const std::string mReferenceFile;
    };

    /* Perform an http request and checks the result. If the result is not the expected one,
     * the exception RequestFailureException is thrown
     *
     * @param[in] uri the URI of the request
     * @param[in] verb the http verb
     * @param[in] requestContent the content of the request (as std::string or util::Buffer)
     * @param[in] expectedStatus the expected http status returned by the server
     * @param[in] expectedContentType the expected response content type returned by the server
     * @param[in] expectedResponseContent the expected response content returned by the server. It
     *                                    can be either an AnyContent, a StringContent or a
     *                                    FileContent
     */
    /** @{ */
    void request(const std::string &uri, Verb verb, const std::string &requestContent,
                 Status expectedStatus, const std::string &expectedContentType,
                 const Content &expectedResponseContent);

    void request(const std::string &uri, Verb verb, const util::Buffer &requestContent,
                 Status expectedStatus, const std::string &expectedContentType,
                 const Content &expectedResponseContent);
    /** @} */

private:
    HttpClientSimulator(const HttpClientSimulator &) = delete;
    HttpClientSimulator &operator=(const HttpClientSimulator &) = delete;

    const std::string mServer;
    const uint32_t mPort;
};
}
}
