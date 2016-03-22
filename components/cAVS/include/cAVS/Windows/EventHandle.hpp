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

    virtual ~EventHandle() {}
    EventHandle() = default;
    EventHandle(const EventHandle &) = delete;
    EventHandle &operator=(const EventHandle &) = delete;

    /** @returns the underlying windows event handle */
    virtual HANDLE handle() const = 0;

    /** Threadsafe: Will force an exit of wait(). */
    virtual void stopWait() = 0;

    /** Wait for the provided handle to be notified.
    * @return true if the handle was notified.
    *         false if the wait was interrupted by a call to stopWait()
    */
    virtual bool wait() = 0;
};

/** Wrap a windows event handle*/
class SystemEventHandle : public EventHandle
{
public:
    SystemEventHandle()
    {
        HANDLE event = CreateEventA(NULL,  /* default security attributes */
                                    FALSE, /* auto-reset event */
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

    void stopWait() override
    {
        mStopped = true;
        SetEvent(mEventHandle);
    }

    bool wait() override
    {
        // Wait for any handle to be notified
        auto status = WaitForSingleObject(mEventHandle, INFINITE);

        bool stopped = mStopped; // Did we exit due to a stop request ?
        mStopped = false;

        switch (status) {
        case WAIT_FAILED:
            throw Exception("Wait for handle failed: " + LastError::get());
        case WAIT_TIMEOUT:
            ASSERT_ALWAYS(false); // INFINITE should not timeout
        case WAIT_OBJECT_0:
            return not stopped;
        };
        throw Exception("Unsupported WaitForSingleObject return code: " + std::to_string(status));
    }

private:
    HANDLE mEventHandle;
    std::atomic<bool> mStopped = {false};
};
}
}
}
