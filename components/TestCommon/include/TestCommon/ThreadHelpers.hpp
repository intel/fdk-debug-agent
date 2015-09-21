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
#include <thread>
#include <condition_variable>
#include <mutex>
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
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    /** Ensure that the supplied function ends before a given timemout.
     *
     * The supplied function is executed into a dedicated thread.
     * - if the function returns before the timeout, the associated thread ends and is joined.
     * - if the timemout is reached, the ensureNonBlocking method throws an exception,
     *   and the thread is not joined, and may run undefinitely, leading to potential memory leaks.
     *
     * That's why this method should only be used in test context.
     */
    static void ensureNonBlocking(std::function<void()> function,
        std::size_t timeoutMs = 5000)
    {
        std::mutex mutex;
        std::condition_variable var;
        std::cv_status status;

        /* Starting a locked context */
        std::unique_lock<std::mutex> locker(mutex);

        /* Starting the thread that will execute the user function. The operator 'new' is used
         * explicitly since the Thread cannot be deleted in case we have no other choice than
         * abandon it. */
        std::thread *thread = new std::thread(functionThread,
            function, std::ref(mutex), std::ref(var));

        /* The mutex will be released by the wait_for() method, and only after that, the thread
         * that executes the user function will be able to notify the condition variable, making
         * return the wait_for() method.
         */
        status = var.wait_for(locker, std::chrono::milliseconds(timeoutMs));

        if (status == std::cv_status::timeout) {
            /* The thread is not terminated, so abandoning it and throwing an exception */
            throw Exception("Unable to join thread: timeout reached.");
        }
        else {
            /* The function has returned before the timeout: joining and deleting the thread. */
            thread->join();
            delete thread;
        }
    }

private:
    ThreadHelpers() = delete;

    /** Launched in a dedicated thread */
    static void functionThread(std::function<void()> function,
        std::mutex &mutex, std::condition_variable &var)
    {
        /* Executing user function */
        function();

        /* Entering into a locked context, in this way it is ensured that the client thread is
        locked on var.wait_for() */
        std::unique_lock<std::mutex> locker(mutex);

        /* Waking up the client thread locked on var.wait_for() */
        var.notify_one();
    }
};

}
}