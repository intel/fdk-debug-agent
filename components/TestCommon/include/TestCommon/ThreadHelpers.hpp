/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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
