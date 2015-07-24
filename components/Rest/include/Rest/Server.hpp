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
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };


    /** @param[in] dispatcher The dispatcher that will be used by the server to resolve the
     * resources
     *
     * @throw Server::Exception
     */
    Server(std::shared_ptr<const Dispatcher> dispatcher, uint32_t port);
    ~Server();

private:
    Server(const Server&) = delete;
    Server &operator=(const Server &) = delete;

    Poco::Net::ServerSocket mServerSocket;
    Poco::ThreadPool mThreadPool;
    Poco::Net::HTTPServer mHttpServer;
};

}
}


