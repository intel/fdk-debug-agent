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

#include <Poco/ErrorHandler.h>
#include <Poco/Net/NetException.h>
#include <Poco/Exception.h>
#include <iostream>

namespace debug_agent
{
namespace rest
{

/**
* Implements a Poco ErrorHandler that hides IOException (and derived exceptions as
* NetException) because these exceptions are thrown when a client disconnects, or when
* the server closes all its sockets.
*/
class ErrorHandler final : public Poco::ErrorHandler
{
public:
    ErrorHandler() : mBaseErrorHandler(nullptr)
    {
        /* Getting base handler */
        mBaseErrorHandler = Poco::ErrorHandler::get();
        poco_check_ptr(mBaseErrorHandler);

        /* Setting this as current handler*/
        Poco::ErrorHandler::set(this);
    }

    ~ErrorHandler()
    {
        /* Restoring base handler */
        Poco::ErrorHandler::set(mBaseErrorHandler);
    }

    virtual void exception(const Poco::Exception &e) override
    {
        /* comparing the exception type */
        if (dynamic_cast<const Poco::IOException *>(&e) != nullptr) {

            /* IOException occurs during client disconnection/server closing
            * It's a nominal case, so hiding it.
            */
            return;
        }

        /* Calling base handler */
        mBaseErrorHandler->exception(e);
    }

private:
    Poco::ErrorHandler *mBaseErrorHandler;
};
}
}
