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

namespace debug_agent
{
namespace rest
{

/**
 * Describe a REST stream response for custom body data
 * A CustomResponse is intended to be inherited to specialize the wat the HTTP body data is
 * provided by the Rest Resource.
 * Hence, the subclasses doBodyResponse() method will be invoked with a std::ostream to which
 * the HTTP body data can be written to.
 * @todo Once C++14 is fully supported by compilers used for Debug Agent, and especially the lambda
 * generalized capture, this class shall be final, and the doBodyResponse method shall be provided
 * as argument to the constructor.
 */
class CustomResponse: public Response
{
public:
    using base = Response;

    /**
     * @param[in] contentType The HTTP MIME type corresponding to the HTTP body data
     */
    explicit CustomResponse(const std::string &contentType):
        base(contentType)
    {
    }

    CustomResponse() = delete;

    virtual void sendHttpBody() override final
    {
        try
        {
            base::sendHttpBody();
            doBodyResponse(*mOut);
        }
        catch (std::exception &e)
        {
            throw HttpAbort(
                std::string("HTTP abort due to exception: ") + typeid(e).name() + ": " + e.what());
        }
    }

protected:
    /**
     * This method has to be implemented by the subclasse to implement a specific HTTP body data
     * generation. The method is called after the HTTP header has been sent to the client, hence
     * only Response::HttpAbort can be raised from this method.
     * @param[in] out The std::ostream to be used to stream out HTTP body data to the client
     * @throw Response::HttpAbort
     */
    virtual void doBodyResponse(std::ostream &out) = 0;
};

}
}


