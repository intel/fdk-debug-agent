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

    class RequestException : public std::exception
    {
    public:
        RequestException(ErrorStatus status, const std::string &userMessage = "") :
            mMessage(getErrorMessage(status, userMessage).c_str()), mStatus(status) {}

        ErrorStatus getStatus() { return mStatus; }

        /* @todo 'throw()' is deprecated with c++11, it should be replaced by 'noexcept'. But
         * this keyword is not supported by visual studio 2013 yet. So keeping 'throw()' */
        virtual const char *what() const throw() { return mMessage.c_str(); }

    private:
        std::string mMessage;
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

    /* Handle a REST request and returns a REST response.
     * @throw RequestException
     */
    virtual void handleRequest(const Request &request, Response &response) = 0;

private:
    Resource(const Resource&) = delete;
    Resource &operator=(const Resource &) = delete;
};

}
}


