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

#include "Rest/Response.hpp"
#include <Poco/MemoryStream.h>
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
class StreamResponse final: public Response
{
public:
    using base = Response;

    /**
     * This constructor will raise an exception in case the contentType is empty.
     * @param[in] contentType MIME type corresponding to body type
     * @param[in] istream response body data to be sent
     * @throw Response::HttpError
     */
    explicit StreamResponse(const std::string &contentType, std::unique_ptr<std::istream> istream):
        base(contentType),
        mIstream(std::move(istream))
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
        try
        {
            base::sendHttpBody();
            Poco::StreamCopier::copyStream(*mIstream, *mOut);
        }
        catch (std::exception &e)
        {
            throw HttpAbort(
                std::string("HTTP abort due to exception: ") + typeid(e).name() + ": " + e.what());
        }
    }

private:
    std::unique_ptr<std::istream> mIstream;
};

}
}
