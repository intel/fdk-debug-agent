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
#include "Rest/Resource.hpp"

namespace debug_agent
{
namespace rest
{

Resource::ResponsePtr Resource::handleRequest(const Request &request)
{
    switch (request.getVerb()) {
    case Request::Verb::Get:
        return handleGet(request);
    case Request::Verb::Put:
        return handlePut(request);
    case Request::Verb::Post:
        return handlePost(request);
    case Request::Verb::Delete:
        return handleDelete(request);
    }
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}

Resource::ResponsePtr Resource::handleGet(const Request &)
{
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}

Resource::ResponsePtr Resource::handlePut(const Request &)
{
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}

Resource::ResponsePtr Resource::handlePost(const Request &)
{
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}

Resource::ResponsePtr Resource::handleDelete(const Request &)
{
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}
}
}
