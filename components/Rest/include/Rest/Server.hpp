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

#pragma once

#include "Rest/Dispatcher.hpp"
#include <Poco/Net/HTTPServer.h>
#include <Poco/ThreadPool.h>

namespace debug_agent
{
namespace rest
{

/** Implement a REST server using the Poco http library
 *
 * For more infortion about REST servers, see:
 * http://en.wikipedia.org/wiki/Representational_state_transfer
 *
 * Note: the Poco API is hidden, in this way the underlying http library can be changed
 * without impacting the client.
 */
class Server
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** @param[in] dispatcher The dispatcher that will be used by the server to resolve the
     * resources
     *
     * @throw Server::Exception
     */
    Server(std::unique_ptr<const Dispatcher> dispatcher, uint32_t port, bool isVerbose = false);
    ~Server();

private:
    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;

    Poco::Net::ServerSocket mServerSocket;
    Poco::ThreadPool mThreadPool;
    Poco::Net::HTTPServer mHttpServer;
};
}
}
