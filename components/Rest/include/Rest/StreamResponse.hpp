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

#include "Rest/Response.hpp"
#include <Poco/StreamCopier.h>
#include <memory>
#include <istream>

namespace debug_agent
{
namespace rest
{

/**
 * Describe a REST HTTP response for std::istream body data
 * A response is an HTTP status to be sent in the HTTP header and an HTTP body to be sent
 * subsequently. The HTTP body data are provided by a std::istream.
 */
class StreamResponse final : public Response
{
public:
    using base = Response;

    /**
     * This constructor will raise an exception in case the contentType is empty.
     * @param[in] contentType MIME type corresponding to body type
     * @param[in] istream response body data to be sent
     * @throw Response::HttpError
     */
    explicit StreamResponse(const std::string &contentType, std::unique_ptr<std::istream> istream)
        : base(contentType), mIstream(std::move(istream))
    {
        if (mContentType.empty()) {

            throw HttpError(ErrorStatus::InternalError, "Missing HTTP Content-Type");
        }
        if (mIstream == nullptr) {

            throw HttpError(ErrorStatus::InternalError, "IStream is null");
        }
    }

    StreamResponse() = delete;

    virtual void sendHttpBody() override
    {
        try {
            base::sendHttpBody();
            Poco::StreamCopier::copyStream(*mIstream, *mOut);
        } catch (std::exception &e) {
            throw HttpAbort(std::string("HTTP abort due to exception: ") + typeid(e).name() + ": " +
                            e.what());
        }
    }

private:
    std::unique_ptr<std::istream> mIstream;
};
}
}
