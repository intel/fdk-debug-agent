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

#include <Poco/StreamCopier.h>
#include <Poco/Net/HTTPRequest.h>
#include <iostream>
#include <map>
#include <assert.h>

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
        switch (verb)
        {
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
        assert(it != mIdentifiers.end());

        return it->second;
    }

private:
    friend class RestResourceRequestHandler;

    /* Constructor is called by the RestResourceRequestHandler class */
    Request(Verb verb, std::istream &requestStream, const Identifiers &identifiers) :
        mVerb(verb), mRequestStream(requestStream), mIdentifiers(identifiers) {}

    Request(const Request&) = delete;
    Request &operator=(const Request &) = delete;

    Verb mVerb;
    std::istream &mRequestStream;
    Identifiers mIdentifiers;
};

}
}


