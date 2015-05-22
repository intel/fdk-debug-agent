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

#include "Rest/Resource.hpp"
#include <iostream>
#include <regex>
#include <memory>
#include <string>
#include <exception>
#include <map>

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
    class InvalidUriException : public std::exception
    {
    public:
        InvalidUriException(const std::string &msg) : mMessage(msg.c_str()) {}

        /* @todo 'throw()' is deprecated with c++11, it should be replaced by 'noexcept'. But
         * this keyword is not supported by visual studio 2013 yet. So keeping 'throw()' */
        virtual const char *what() const throw() { return mMessage.c_str(); }

    private:
        std::string mMessage;
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
    std::shared_ptr<Resource> resolveResource(const std::string &uri, Identifiers &identifiers)
        const;

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

    using ResourceEntryCollection = std::vector< ResourceEntry >;

    Dispatcher(const Dispatcher&) = delete;
    Dispatcher &operator=(const Dispatcher &) = delete;

    ResourceEntryCollection mResourceEntryCollection;
};

}
}


