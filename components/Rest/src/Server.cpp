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

#include "ServerRequestHandling.hpp"
#include "Rest/Server.hpp"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NetException.h"
#include <cassert>

using namespace Poco::Net;

namespace debug_agent
{
namespace rest
{
/* Poco forces us to use operator new here: the HttpServer takes the ownership of the
 * RequestHandlerFactory and the HTTPServerParams.
 */
Server::Server(std::unique_ptr<const Dispatcher> dispatcher, uint32_t port, bool isVerbose)
try: mServerSocket(port),
     mHttpServer(new RequestHandlerFactory(std::move(dispatcher), isVerbose),
                 mThreadPool, mServerSocket, new HTTPServerParams())
{
    mHttpServer.start();
}
catch (Poco::Net::NetException &e)
{
    throw Exception("Unable to start http server: " + std::string(e.what()));
}

Server::~Server()
{
    try
    {
        /* Stopping the http server immediately, leading to close current http connections.
        * The http server cannot wait that all client disconnect. If one client is doing
        * streaming (log retrieval...) the server would never be able to close.
        */
        mHttpServer.stopAll(true);
    }
    catch (Poco::Net::NetException &e)
    {
        /* Exception is swallowed because there is no way to handle the issue */
        /* @todo: use logging */
        std::cout << "Unable to stop http server: " << e.what() << std::endl;
    }

    /* Now all sockets are closed, the http request threads should finish. Joining them */
    mThreadPool.joinAll();
}

}
}
