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

#include "Util/AssertAlways.hpp"
#include <condition_variable>
#include <mutex>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

class SyncWait
{
public:
    /** Block until someone calls the unblockWait() method */
    void waitUntilUnblock()
    {
        std::unique_lock<std::mutex> locker(mWaitVarMutex);
        if (not mWaiting && not mUnblocked) {
            mWaiting = true;
            mWaitVar.wait(locker);
        }
        ASSERT_ALWAYS(!mWaiting);
    }

    void unblockWait()
    {
        std::unique_lock<std::mutex> locker(mWaitVarMutex);
        if (mWaiting) {
            mWaiting = false;
            mUnblocked = true;
            mWaitVar.notify_one();
        }
    }

private:
    std::condition_variable mWaitVar;
    std::mutex mWaitVarMutex;
    bool mWaiting = false;
    bool mUnblocked = false;
};
}
}
}
