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

#include "Util/AssertAlways.hpp"
#include <Poco/Net/HTTPServerResponse.h>
#include <sstream>
#include <iostream>

namespace debug_agent
{
namespace rest
{

/**
 * Describe a REST HTTP response with optional simple body data
 * A response is an HTTP status to be sent in the HTTP header and an optional HTTP body to be sent
 * subsequently.
 */
class Response
{
public:
    enum class ErrorStatus
    {
        NotFound = 404,
        BadRequest = 400,
        VerbNotAllowed = 405,
        Locked = 423,
        InternalError = 500
    };

    static std::string toString(ErrorStatus status)
    {
        switch (status) {
        case ErrorStatus::NotFound:
            return "Resource not found";
        case ErrorStatus::BadRequest:
            return "Bad request";
        case ErrorStatus::VerbNotAllowed:
            return "Verb not allowed";
        case ErrorStatus::Locked:
            return "Resource is locked";
        case ErrorStatus::InternalError:
            return "Internal error";
        }
        abort();
    }

    /**
     * HttpError exception
     * This exception is intended to be raised when an HTTP request cannot be handled. In such
     * case a HTTP header response including the HTTP error status and the error message shall be
     * sent as response to the client.
     */
    class HttpError : public std::logic_error
    {
    public:
        /**
         * @param[in] status The HTTP error status to be sent in the HTTP response header
         * @param[in] userMessage The message to be carried on by the exception object which will
         * be in the body of the HTTP error response
         */
        HttpError(ErrorStatus status, const std::string &userMessage = "")
            : std::logic_error(getErrorMessage(status, userMessage).c_str()), mStatus(status)
        {
        }

        ErrorStatus getStatus() { return mStatus; }

    private:
        ErrorStatus mStatus;

        static std::string getErrorMessage(ErrorStatus status, const std::string &userMessage)
        {
            std::stringstream msg;
            msg << toString(status);
            if (userMessage.length() > 0) {
                msg << ": " << userMessage;
            }
            return msg.str();
        }
    };

    /**
     * HttpAbort exception
     * This exception is intended to be raised when an error occurs after the HTTP response header
     * has been sent and while HTTP response body is being sent. The HTTP connection of the aborting
     * request shall just be abandoned and closed.
     */
    class HttpAbort : public std::logic_error
    {
    public:
        /**
         * @param[in] what The message to be carried on by the exception object
         */
        explicit HttpAbort(const std::string &what) : std::logic_error(what) {}
    };

    /**
     * Construct a Response for HTTP status OK with a body
     * @param[in] contentType MIME type corresponding to body type
     * @param[in] responseBody response body
     */
    explicit Response(const std::string &contentType, const std::string &responseBody)
        : mContentType(contentType), mOut(nullptr), mContent(responseBody){};

    /**
     * Construct a simple Response for HTTP status OK without any more data
     */
    Response() : mContentType(), mOut(nullptr), mContent() {}

    virtual ~Response() {}

    /**
     * Send the HTTP response header to the client
     * @param[in] serverResponse the Poco HTTP server response object through which the Response
     * has to be sent out.
     */
    void sendHttpHeader(Poco::Net::HTTPServerResponse &serverResponse)
    {
        setCommonProperties(serverResponse);
        serverResponse.setContentType(mContentType);
        serverResponse.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);

        mOut = &serverResponse.send();
    }

    /**
     * Send the HTTP response body to the client
     * @throw Response::HttpAbort
     */
    virtual void sendHttpBody()
    {
        ASSERT_ALWAYS(mOut != nullptr);

        *mOut << mContent;
    }

    static void sendHttpError(Poco::Net::HTTPResponse::HTTPStatus status,
                              const std::string &message, Poco::Net::HTTPServerResponse &resp)
    {
        /* @todo Use logging instead */
        std::cout << "Send HTTP error " << status << ": " << message << std::endl;

        setCommonProperties(resp);
        resp.setStatus(status);
        resp.setContentType("text/plain");

        try {
            std::ostream &out = resp.send();
            out << message;
        } catch (std::exception &e) {
            /* @todo Use logging instead */
            std::cout << "Failed to send HTTP error " << status << ": " << e.what() << std::endl;
        }
    }

protected:
    /**
     * Construct a Response for HTTP status OK with a body content type, but without body data.
     * This constructor is provided for subclasses which will manage the body data by themselves.
     * @param[in] contentType MIME type corresponding to body type
     */
    explicit Response(const std::string &contentType)
        : mContentType(contentType), mOut(nullptr), mContent(){};

private:
    /**
     * A method helper which provide a common way to set common HTTP property to all Response
     */
    static void setCommonProperties(Poco::Net::HTTPMessage &httpMessage)
    {
        httpMessage.setChunkedTransferEncoding(true);
        httpMessage.setKeepAlive(true);
    }

    Response(const Response &) = delete;
    Response &operator=(const Response &) = delete;

protected:
    std::string mContentType;
    std::ostream *mOut;

private:
    std::string mContent;
};
}
}
