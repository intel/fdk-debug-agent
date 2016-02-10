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
#include <stdexcept>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Wrap a windows event handle*/
class EventHandle
{
public:
    class Exception : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    EventHandle()
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

    EventHandle(EventHandle &&other)
    {
        mEventHandle = other.mEventHandle;
        other.mEventHandle = NULL;
    }

    ~EventHandle()
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

    HANDLE get() const { return mEventHandle; }

private:
    EventHandle(const EventHandle &) = delete;
    EventHandle &operator=(const EventHandle &) = delete;

    HANDLE mEventHandle;
};
}
}
}
