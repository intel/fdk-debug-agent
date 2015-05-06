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

#include "Core/DebugAgent.hpp"
#include <Poco/Util/Application.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/NetException.h>
#include <iostream>

using namespace Poco::Util;
using namespace Poco::Net;

namespace debug_agent
{
namespace core
{

/** Handle resource not found case */
class UnknownResourceRequestHandler : public HTTPRequestHandler
{
public:
    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
    {
        resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
        resp.setContentType("text/plain");

        try
        {
            std::ostream& out = resp.send();
            out << "Resource not found." << std::endl;
        }
        catch (ConnectionAbortedException &)
        {
            std::cout << "Connection aborted." << std::endl;
        }
    }
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &req)
    {
        /* For now just returning 404 http result code (page not found) */
        return new UnknownResourceRequestHandler;
    }
};

int DebugAgent::main(const std::vector<std::string>&)
{
    HTTPServerParams::Ptr params = new HTTPServerParams();

    HTTPServer s(new RequestHandlerFactory, ServerSocket(9090), params);
    s.start();
    std::cout << "DebugAgent started" << std::endl;

    waitForTerminationRequest();  /* wait for CTRL-C or kill */

    std::cout << std::endl << "Shutting down DebugAgent..." << std::endl;
    s.stop();

    return Application::EXIT_OK;
}

}
}
