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

#include <Util/AssertAlways.hpp>
#include <Poco/StreamCopier.h>
#include <Poco/Net/HTTPRequest.h>
#include <iostream>
#include <map>
#include <cassert>

namespace debug_agent
{
namespace rest
{

/** Describe a REST request
 *
 * A rest request contains:
 * - a verb (Get, Post..)
 * - a request content (as an input stream)
 * - the identifier values (ex: "account-id=12")
 */
class Request final
{
public:
    using Identifiers = std::map<std::string, std::string>;

    enum class Verb
    {
        Get,
        Post,
        Put,
        Delete
    };

    static std::string toString(Verb verb)
    {
        switch (verb) {
        case Verb::Get:
            return "GET";
        case Verb::Post:
            return "POST";
        case Verb::Put:
            return "PUT";
        case Verb::Delete:
            return "DELETE";
        }
        abort();
    }

    Verb getVerb() const { return mVerb; }
    std::istream &getRequestStream() const { return mRequestStream; }
    std::string getRequestContentAsString() const
    {
        std::string content;
        Poco::StreamCopier::copyToString(getRequestStream(), content);
        return content;
    }
    const Identifiers &getIdentifiers() const { return mIdentifiers; }

    std::string getIdentifierValue(const std::string &identifier) const
    {
        Identifiers::const_iterator it = mIdentifiers.find(identifier);
        ASSERT_ALWAYS(it != mIdentifiers.end());

        return it->second;
    }

private:
    friend class RestResourceRequestHandler;

    /* Constructor is called by the RestResourceRequestHandler class */
    Request(Verb verb, std::istream &requestStream, const Identifiers &identifiers)
        : mVerb(verb), mRequestStream(requestStream), mIdentifiers(identifiers)
    {
    }

    Request(const Request &) = delete;
    Request &operator=(const Request &) = delete;

    Verb mVerb;
    std::istream &mRequestStream;
    Identifiers mIdentifiers;
};
}
}
