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
#include "Rest/Resource.hpp"

namespace debug_agent
{
namespace rest
{

void Resource::handleRequest(const Request &request, Response &response)
{
    switch (request.getVerb())
    {
    case Request::Verb::Get:
        handleGet(request, response);
        return;
    case Request::Verb::Put:
        handlePut(request, response);
        return;
    case Request::Verb::Post:
        handlePost(request, response);
        return;
    case Request::Verb::Delete:
        handleDelete(request, response);
        return;
    }
    throw RequestException(ErrorStatus::VerbNotAllowed);
}

void Resource::handleGet(const Request &request, Response &response)
{
    throw RequestException(ErrorStatus::VerbNotAllowed);
}

void Resource::handlePut(const Request &request, Response &response)
{
    throw RequestException(ErrorStatus::VerbNotAllowed);
}

void Resource::handlePost(const Request &request, Response &response)
{
    throw RequestException(ErrorStatus::VerbNotAllowed);
}

void Resource::handleDelete(const Request &request, Response &response)
{
    throw RequestException(ErrorStatus::VerbNotAllowed);
}

}
}