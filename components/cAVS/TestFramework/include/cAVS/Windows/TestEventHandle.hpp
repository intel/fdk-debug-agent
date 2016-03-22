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

#include "cAVS/Windows/EventHandle.hpp"
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
    /** Raise the event and blocks until someone calls the wait() method */
    void raiseEventAndBlockUntilWait()
    {
        std::unique_lock<std::mutex> locker(mWaitVarMutex);
        SetEvent(handle());
        mWaitVar.wait(locker);
    }

    bool wait() override
    {
        {
            std::unique_lock<std::mutex> locker(mWaitVarMutex);
            mWaitVar.notify_one();
        }
        return Base::wait();
    }

private:
    using Base = SystemEventHandle;
    std::condition_variable mWaitVar;
    std::mutex mWaitVarMutex;
};
}
}
}