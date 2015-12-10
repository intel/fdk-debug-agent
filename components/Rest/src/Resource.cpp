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

Resource::ResponsePtr Resource::handleRequest(const Request &request)
{
    switch (request.getVerb()) {
    case Request::Verb::Get:
        return handleGet(request);
    case Request::Verb::Put:
        return handlePut(request);
    case Request::Verb::Post:
        return handlePost(request);
    case Request::Verb::Delete:
        return handleDelete(request);
    }
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}

Resource::ResponsePtr Resource::handleGet(const Request &request)
{
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}

Resource::ResponsePtr Resource::handlePut(const Request &request)
{
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}

Resource::ResponsePtr Resource::handlePost(const Request &request)
{
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}

Resource::ResponsePtr Resource::handleDelete(const Request &request)
{
    throw Response::HttpError(Response::ErrorStatus::VerbNotAllowed);
}
}
}