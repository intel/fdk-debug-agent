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

#include "cAVS/Windows/EventHandle.hpp"
#include "Util/AssertAlways.hpp"
#include <condition_variable>
#include <mutex>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Specialized event handle that provides the ability to raise the event and block until
 * someone is waiting */
class TestEventHandle : public SystemEventHandle
{
public:
    /** Block until someone calls the wait() method */
    void blockUntilWait()
    {
        std::unique_lock<std::mutex> locker(mWaitVarMutex);
        if (!mWaiting) {
            mWaitVar.wait(locker);
        }
        ASSERT_ALWAYS(mWaiting);
        mWaiting = false;
    }

    void wait() override
    {
        {
            std::unique_lock<std::mutex> locker(mWaitVarMutex);
            mWaiting = true;
            mWaitVar.notify_one();
        }
        Base::wait();
    }

private:
    using Base = SystemEventHandle;
    std::condition_variable mWaitVar;
    std::mutex mWaitVarMutex;
    bool mWaiting = false;
};
}
}
}