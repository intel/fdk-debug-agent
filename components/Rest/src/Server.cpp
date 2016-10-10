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
Server::Server(std::unique_ptr<const Dispatcher> dispatcher, uint32_t port, bool isVerbose) try
    : mServerSocket(port),
      mHttpServer(new RequestHandlerFactory(std::move(dispatcher), isVerbose), mThreadPool,
                  mServerSocket, new HTTPServerParams()) {
    mHttpServer.start();
} catch (Poco::Net::NetException &e) {
    throw Exception("Unable to start http server: " + std::string(e.what()));
}

Server::~Server()
{
    try {
        /* Stopping the http server immediately, leading to close current http connections.
        * The http server cannot wait that all client disconnect. If one client is doing
        * streaming (log retrieval...) the server would never be able to close.
        */
        mHttpServer.stopAll(true);
    } catch (Poco::Net::NetException &e) {
        /* Exception is swallowed because there is no way to handle the issue */
        /* @todo: use logging */
        std::cout << "Unable to stop http server: " << e.what() << std::endl;
    }

    /* Now all sockets are closed, the http request threads should finish. Joining them */
    mThreadPool.joinAll();
}
}
}
