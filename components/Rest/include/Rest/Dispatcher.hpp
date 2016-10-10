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

#include "Rest/Resource.hpp"
#include <iostream>
#include <regex>
#include <memory>
#include <string>
#include <exception>
#include <map>
#include <stdexcept>

namespace debug_agent
{
namespace rest
{

/** The Dispatcher intends to find a rest resource using a supplied URI
 *
 * Resources are inserted into the dispatcher using a reference URI, which may contain
 * some resource identifiers, for instance:
 *
 * /accounts/${account_id}
 *
 * "account_id" is a resource identifier.
 *
 * When the dispatcher resolves an URI, it fetches the identifier value. For instance when this URI
 * is resolved:
 *
 * /accounts/jim
 *
 * The dispatcher will understand that 'jim' is the value of the identifier 'account-id'.
 */
class Dispatcher final
{
public:
    class InvalidUriException : public std::logic_error
    {
    public:
        InvalidUriException(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    using Identifiers = std::map<std::string, std::string>;

    Dispatcher();
    ~Dispatcher();

    /** Add a resource associated to an URI.
     *
     * @param[in] uriWithIdentifers the resource URI. It can contain identifiers,
     *            for instance ${probe-id}.
     * @param[in] resource the REST resource
     * @throw InvalidUriException if the supplied URI is incorrect.
     */
    void addResource(const std::string &uriWithIdentifers, std::shared_ptr<Resource> resource);

    /** Resolve a resource using a supplied URI
     * @param[in] uri the resource URI
     * @param[out] identifiers The fetched identifiers
     * @return the resolved resource, or nullptr if not found. */
    std::shared_ptr<Resource> resolveResource(const std::string &uri,
                                              Identifiers &identifiers) const;

private:
    /* Contain information related to a resource */
    class ResourceEntry
    {
    public:
        /* May throw InvalidUriException */
        ResourceEntry(const std::string &uri, std::shared_ptr<Resource> resource);

        /* Returns true if the resource entry matches the supplied URI
         * @param[in] uri the candidate URI
         * @param[out] identifiers The fetched identifiers
         * @return true is the entry matches the uri. The identifiers are filled accordingly.
         */
        bool matchUri(const std::string &uri, Identifiers &identifiers) const;
        std::shared_ptr<Resource> getResource() const { return mResource; }

    private:
        /* The regular expression used to resolve the entry */
        std::regex mUriRegExp;

        /* Contains optional identifier names. For instance the URI /accounts/${account-id}
         * has one identifier 'account-id'
         * will contain the identifier name "account-id"
         */
        std::vector<std::string> mIndentifierNames;

        /* The matching REST resource*/
        std::shared_ptr<Resource> mResource;
    };

    using ResourceEntryCollection = std::vector<ResourceEntry>;

    Dispatcher(const Dispatcher &) = delete;
    Dispatcher &operator=(const Dispatcher &) = delete;

    ResourceEntryCollection mResourceEntryCollection;
};
}
}
