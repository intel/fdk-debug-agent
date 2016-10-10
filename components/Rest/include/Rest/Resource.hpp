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
    Resource(const Resource &) = delete;
    Resource &operator=(const Resource &) = delete;
};
}
}
