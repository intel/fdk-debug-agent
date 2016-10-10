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

#include "Rest/Dispatcher.hpp"
#include <iostream>
#include <cassert>

namespace debug_agent
{
namespace rest
{

/* Reg exp that identifies a symbol, for instance 'aa_b2' */
static const std::string SymbolExp("([a-zA-Z0-9_\\.\\-]+)");

static const std::string ResourceIdentifierPrefix("\\$\\{");
static const size_t ResourceIdentifierPrefixLength = 2;

static const std::string ResourceIdentifierSuffix("\\}");
static const size_t ResourceIdentifierSuffixLength = 1;

/* Reg exp that identifies a resource identifier, for instance '${account_id}' */
static const std::string ResourceIdentifierExp(ResourceIdentifierPrefix + SymbolExp +
                                               ResourceIdentifierSuffix);

/* Reg exp that identifies a single path node of an URI, which can be a symbol or a
    * resource identifier */
static const std::string SinglePathExp(std::string("(") + ResourceIdentifierExp + "|" + SymbolExp +
                                       ")");

/* Reg exp that identifies an URI (with several path nodes separated by '/') */
static const std::string UriExp(std::string("(/") + SinglePathExp + ")+");

static const std::regex ResourceIdentifierRegExp(ResourceIdentifierExp);
static const std::regex UriRegExp(UriExp);

Dispatcher::ResourceEntry::ResourceEntry(const std::string &uri, std::shared_ptr<Resource> resource)
    : mResource(resource)
{
    assert(mResource != nullptr);

    /* root special case*/
    if (uri == "/") {
        mUriRegExp = uri;
        return;
    }

    /* Checking uri syntax */
    if (!std::regex_match(uri, UriRegExp)) {
        throw InvalidUriException("Wrong URI: " + uri + " Regexp used: " + UriExp);
    }

    /* Finding resource identifiers, for instance '${account_id}' */
    std::smatch match;
    std::string currentUri = uri;
    while (std::regex_search(currentUri, match, ResourceIdentifierRegExp)) {
        if (match.size() > 0) {

            /* contain the resource identifer expression, i.e. '${account_id}' */
            std::string resourceIdentifierExp = match.str(0);

            /* contain the resource identifier name, i.e. 'account_id' */
            std::string resourceIdentifierName = resourceIdentifierExp.substr(
                ResourceIdentifierPrefixLength,
                resourceIdentifierExp.length() -
                    (ResourceIdentifierPrefixLength + ResourceIdentifierSuffixLength));

            /* checking that the resource identifer does not already exist' */
            if (std::find(mIndentifierNames.begin(), mIndentifierNames.end(),
                          resourceIdentifierName) != mIndentifierNames.end()) {
                throw InvalidUriException("Wrong URI '" + uri + "' : The identifier '" +
                                          resourceIdentifierName + " is not unique.");
            }

            /* Adding the resource identifier name */
            mIndentifierNames.push_back(resourceIdentifierName);
        }
        currentUri = match.suffix().str();
    }

    /* Replacing identifiers by symbol capture groups. Each capture group will allow to
     * find the value of each identifier */
    currentUri = std::regex_replace(uri, ResourceIdentifierRegExp, SymbolExp);
    currentUri += "/?"; /* optional trailing slash*/

    mUriRegExp = currentUri;
}

bool Dispatcher::ResourceEntry::matchUri(const std::string &uri, Identifiers &identifiers) const
{
    /* Using the reg exp to determine if the supplied URI matches the reference URI*/
    std::smatch match;
    if (!std::regex_match(uri, match, mUriRegExp))
        return false;

    /* Iterating on capture groups to find the identifier values
     * Note the first capture group is always the whole string and should be ignored, so
     * that's why capture_group_count = identifier_count + 1 */
    /* @todo: replace assert by always_assert */
    assert(match.size() == (mIndentifierNames.size() + 1));

    for (size_t i = 0; i < mIndentifierNames.size(); i++) {
        identifiers[mIndentifierNames[i]] = match.str(i + 1);
    }
    return true;
}

Dispatcher::Dispatcher()
{
}

Dispatcher::~Dispatcher()
{
}

void Dispatcher::addResource(const std::string &uriWithIdentifers,
                             std::shared_ptr<Resource> resource)
{
    mResourceEntryCollection.push_back(ResourceEntry(uriWithIdentifers, resource));
}

std::shared_ptr<Resource> Dispatcher::resolveResource(const std::string &uri,
                                                      Identifiers &identifiers) const
{
    identifiers.clear();

    for (const auto &resourceEntry : mResourceEntryCollection) {
        if (resourceEntry.matchUri(uri, identifiers)) {
            return resourceEntry.getResource();
        }
    }
    return nullptr;
}
}
}
