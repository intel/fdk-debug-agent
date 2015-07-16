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
#include "cAVS/Windows/RealTimeWppClient.hpp"
#include "cAVS/Windows/WppConsumer.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

RealTimeWppClient::RealTimeWppClient() : mStopRequest(false)
{

}

void RealTimeWppClient::collectLogEntries(WppLogEntryListener &listener)
{
    try
    {
        /* If the stop() occurs before this locked block, the method returns */
        std::lock_guard<std::mutex> locker(mStopRequestMutex);
        if (mStopRequest) {
            return;
        }
        mController.start();

        /* If the stop() occurs after this locked block, the session is already started
         * and therefore will be successfully stopped */
    }
    catch (WppController::Exception &e)
    {
        throw Exception("Cannot start wpp controller: " + std::string(e.what()));
    }

    try
    {
        WppConsumer::collectLogEntries(listener);
    }
    catch (WppConsumer::Exception &e)
    {
        mController.stop();
        throw Exception("Cannot collect entries with wpp consumer: " + std::string(e.what()));
    }

    mController.stop();
}

void RealTimeWppClient::stop()
{
    std::lock_guard<std::mutex> locker(mStopRequestMutex);
    if (!mStopRequest) {
        mStopRequest = true;

        mController.stop();
    }
}

}
}
}
