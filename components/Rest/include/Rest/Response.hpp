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

#include <Poco/Net/HTTPServerResponse.h>
#include <iostream>

namespace debug_agent
{
namespace rest
{

/** Describe a REST response
 *
 * The status and the content type is sent to the client using the send() method.
 * The send() method returns an output stream that can be used to send the response
 * content.
 */
class Response final
{
public:
    /* Send the response to the client
     * @param[in] contentType the response content type
     * @return an output stream that can be used to send the response content
     */
    std::ostream &send(const std::string& contentType) {
        mResponse.setChunkedTransferEncoding(true);
        mResponse.setContentType(contentType);
        mResponse.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK);
        return mResponse.send();
    }

private:
    friend class RestResourceRequestHandler;

    /* Constructor is called by the RestResourceRequestHandler class */
    Response(Poco::Net::HTTPServerResponse &response) : mResponse(response) {}

    Response(const Response&) = delete;
    Response &operator=(const Response &) = delete;

    Poco::Net::HTTPServerResponse &mResponse;
};

}
}


