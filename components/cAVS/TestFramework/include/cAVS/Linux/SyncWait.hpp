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
