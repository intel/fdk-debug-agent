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

#include "Rest/Request.hpp"
#include "Rest/Response.hpp"
#include <memory>

namespace debug_agent
{
namespace rest
{

/** Describe an abstract REST resource
 */
class Resource
{
public:
    Resource() {}
    virtual ~Resource() {}

    using ResponsePtr = std::unique_ptr<Response>;

    /**
     * The default request handler which dispatches requests to the default request handlers.
     * This method calls the default handler according to the request verb.
     * @param[in] request The request to be handled
     * @return the Response to be sent to the client
     * @throw HttpError
     * @see handleGet
     * @see handlePut
     * @see handlePost
     * @see handleDelete
     */
    virtual ResponsePtr handleRequest(const Request &request);

protected:
    /**
     * Default GET handler: raises HttpError (ErrorStatus::VerbNotAllowed)
     * This method is intended to be overridden if the Resource subclass handles GET
     * @param[in] request The GET request to be handled
     * @return the Response to be sent to the client
     * @throw HttpError
     */
    virtual ResponsePtr handleGet(const Request &request);

    /**
     * Default PUT handler: raises HttpError (ErrorStatus::VerbNotAllowed)
     * This method is intended to be overridden if the Resource subclass handles PUT
     * @param[in] request The PUT request to be handled
     * @return the Response to be sent to the client
     * @throw HttpError
     */
    virtual ResponsePtr handlePut(const Request &request);

    /**
     * Default POST handler: raises HttpError (ErrorStatus::VerbNotAllowed)
     * This method is intended to be overridden if the Resource subclasses handles POST
     * @param[in] request The POST request to be handled
     * @return the Response to be sent to the client
     * @throw HttpError
     */
    virtual ResponsePtr handlePost(const Request &request);

    /**
     * Default DELETE handler: raises HttpError (ErrorStatus::VerbNotAllowed)
     * This method is intended to be overridden if the Resource subclass handles DELETE
     * @param[in] request The DELETE request to be handled
     * @return the Response to be sent to the client
     * @throw HttpError
     */
    virtual ResponsePtr handleDelete(const Request &request);

private:
    Resource(const Resource&) = delete;
    Resource &operator=(const Resource &) = delete;
};

}
}


