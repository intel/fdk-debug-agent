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

#include "cAVS/Windows/WindowsTypes.hpp"
#include "cAVS/Windows/LastError.hpp"
#include "Util/Exception.hpp"
#include "Util/AssertAlways.hpp"
#include <stdexcept>
#include <iostream>
#include <atomic>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

class EventHandle
{
public:
    using Exception = util::Exception<EventHandle>;

    class Waiter
    {
    public:
        Waiter(EventHandle &handle) : mHandle(handle) {}
        Waiter(Waiter &&other) : mHandle(other.mHandle), mStopped(other.mStopped.load()) {}
        Waiter(const Waiter &) = delete;
        Waiter &operator=(const Waiter &) = delete;

        /** Wait for the provided handle to be notified.
         * @return true if the handle was notified.
         *         false if the wait was interrupted by a call to stopWait()
         */
        bool wait()
        {
            mHandle.wait();
            return not mStopped;
        }

        /** Threadsafe: Will force an exit of wait(). */
        void stopWait()
        {
            mStopped = true;
            mHandle.notify();
        }

    private:
        EventHandle &mHandle;
        std::atomic<bool> mStopped = {false};
    };

    virtual ~EventHandle() {}
    EventHandle() = default;
    EventHandle(const EventHandle &) = delete;
    EventHandle &operator=(const EventHandle &) = delete;

    /** @returns the underlying windows event handle */
    virtual HANDLE handle() const = 0;

protected:
    /** Raise the event */
    virtual void notify() = 0;

    /** Wait for the provided handle to be notified */
    virtual void wait() = 0;
};

/** Wrap a windows event handle*/
class SystemEventHandle : public EventHandle
{
public:
    SystemEventHandle()
    {
        HANDLE event = CreateEventA(NULL,  /* default security attributes */
                                    FALSE, /* FALSE = auto reset event */
                                    FALSE, /* initial state is nonsignaled */
                                    NULL   /* no name */
                                    );
        if (event == NULL) {
            throw Exception("Cannot create event handle: " + LastError::get());
        }
        mEventHandle = event;
    }

    ~SystemEventHandle()
    {
        if (mEventHandle != NULL) {
            /* According to the documentation, "CloseHandle" returns a nonzero value in success
             * case */
            if (CloseHandle(mEventHandle) == 0) {
                /* @todo: use log instead */
                std::cout << "Cannot close event handle : " + LastError::get() + "\n";
            }
        }
    }

    HANDLE handle() const override { return mEventHandle; }

    void notify() override
    {
        if (SetEvent(mEventHandle) != TRUE) {
            std::cout << "Cannot set event: " << LastError::get() << std::endl;
        }
    }

    void wait() override
    {
        // Wait for any handle to be notified
        auto status = WaitForSingleObject(mEventHandle, INFINITE);
        switch (status) {
        case WAIT_FAILED:
            throw Exception("Wait for handle failed: " + LastError::get());
        case WAIT_TIMEOUT:
            ASSERT_ALWAYS(false); // INFINITE should not timeout
        case WAIT_OBJECT_0:
            return;
        };
        throw Exception("Unsupported WaitForSingleObject return code: " + std::to_string(status));
    }

private:
    HANDLE mEventHandle;
};
}
}
}
