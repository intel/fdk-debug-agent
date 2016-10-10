/*
 * Copyright (c) 2016, Intel Corporation
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
#include <future>
#include <stdexcept>
#include <functional>

namespace debug_agent
{
namespace test_common
{

/** Miscellaneous helper functions about multithreading */
class ThreadHelpers final
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** Ensure that the supplied function ends before a given timemout.
     *
     * The supplied function is executed into a dedicated thread.
     * - if the function returns before the timeout, the associated thread ends and is joined.
     * - if the timemout is reached, the ensureNonBlocking method throws an exception,
     *   and the thread is not joined, and may run undefinitely, leading to memory leaks.
     *
     * That's why this method SHALL ONLY BE USED IN TEST CONTEXT.
     */
    static void ensureNonBlocking(std::function<void()> function, std::size_t timeoutMs = 5000)
    {
        using Future = std::future<void>;

        // Starting lambda asynchroneously
        auto future = std::make_unique<Future>(std::async(std::launch::async, function));

        // Waiting with timeout
        std::future_status status = future->wait_for(std::chrono::milliseconds(timeoutMs));

        if (status == std::future_status::timeout) {
            // Timeout is reached: not deleting the "Future" instance in order to not join
            // the underlying thread that may never end.
            future.release();
            throw Exception("Timeout reached (" + std::to_string(timeoutMs) + "ms)");
        }

        // Calling get() in order to throw exception potentially raised by the lambda.
        future->get();
    }

private:
    ThreadHelpers() = delete;
};
}
}
