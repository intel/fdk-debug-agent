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

#include "Request.hpp"
#include "Response.hpp"
#include <exception>
#include <sstream>
#include <stdexcept>

namespace debug_agent
{
namespace rest
{

/** Describe an abstract REST resource
 */
class Resource
{
public:
    enum class ErrorStatus
    {
        NotFound = 404,
        BadRequest = 400,
        VerbNotAllowed = 405
    };

    static std::string toString(ErrorStatus status)
    {
        switch (status)
        {
        case ErrorStatus::NotFound:
            return "Resource not found";
        case ErrorStatus::BadRequest:
            return "Bad request";
        case ErrorStatus::VerbNotAllowed:
            return "Verb not allowed";
        }
        abort();
    }

    class RequestException : public std::logic_error
    {
    public:
        RequestException(ErrorStatus status, const std::string &userMessage = "") :
            std::logic_error(getErrorMessage(status, userMessage).c_str()), mStatus(status) {}

        ErrorStatus getStatus() { return mStatus; }

    private:
        ErrorStatus mStatus;

        static std::string getErrorMessage(ErrorStatus status, const std::string &userMessage)
        {
            std::stringstream msg;
            msg << toString(status);
            if (userMessage.length() > 0) {
                msg << " : " << userMessage;
            }
            return msg.str();
        }
    };

    Resource() {}
    virtual ~Resource() {}

    /* The default request handler which dispatches requests to the default request handlers.
     * This method calls the default handler according to the request verb.
     * @param[in] request The request to be handled
     * @param[in] response The response to be forwarded
     * @throw RequestException
     * @see handleGet
     * @see handlePut
     * @see handlePost
     * @see handleDelete
     */
    virtual void handleRequest(const Request &request, Response &response);

protected:
    /**
     * Default GET handler: raises RequestException (ErrorStatus::VerbNotAllowed)
     * This method is intended to be overridden if the Resource subclass handles GET
     * @param[in] request The GET request to be handled
     * @param[in] response The response to be forwarded
     * @throw RequestException
     */
    virtual void handleGet(const Request &request, Response &response);

    /**
     * Default PUT handler: raises RequestException (ErrorStatus::VerbNotAllowed)
     * This method is intended to be overridden if the Resource subclass handles PUT
     * @param[in] request The PUT request to be handled
     * @param[in] response The response to be forwarded
     * @throw RequestException
     */
    virtual void handlePut(const Request &request, Response &response);

    /**
     * Default POST handler: raises RequestException (ErrorStatus::VerbNotAllowed)
     * This method is intended to be overridden if the Resource subclasses handles POST
     * @param[in] request The POST request to be handled
     * @param[in] response The response to be forwarded
     * @throw RequestException
     */
    virtual void handlePost(const Request &request, Response &response);

    /**
     * Default DELETE handler: raises RequestException (ErrorStatus::VerbNotAllowed)
     * This method is intended to be overridden if the Resource subclass handles DELETE
     * @param[in] request The DELETE request to be handled
     * @param[in] response The response to be forwarded
     * @throw RequestException
     */
    virtual void handleDelete(const Request &request, Response &response);

private:
    Resource(const Resource&) = delete;
    Resource &operator=(const Resource &) = delete;
};

}
}


