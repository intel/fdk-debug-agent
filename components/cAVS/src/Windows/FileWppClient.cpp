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
#include "cAVS/Windows/FileWppClient.hpp"
#include "cAVS/Windows/WppConsumer.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

FileWppClient::FileWppClient(const std::string &fileName) : mFileName(fileName), mStopRequest(false)
{
    if (mFileName.empty()) {
        throw Exception("FileWppClient: supplied log file name is empty.");
    }
}

void FileWppClient::collectLogEntries(WppLogEntryListener &listener)
{
    {
        std::lock_guard<std::mutex> locker(mStopRequestMutex);
        if (mStopRequest) {
            return;
        }
    }

    try {
        WppConsumer::collectLogEntries(listener, mFileName);
    } catch (WppConsumer::Exception &e) {
        throw Exception("Cannot collect entries with wpp consumer: " + std::string(e.what()));
    }
}

void FileWppClient::stop()
{
    std::lock_guard<std::mutex> locker(mStopRequestMutex);
    mStopRequest = true;

    /* If collectLogEntries() is running, it will not be interrupted by stop(), there is no way to
     * do that properly when reading entries from a log file.
     *
     * I have tried CloseTrace(mHandle) but its changes nothing because all entries have already
     * been put into the driver buffer.
     *
     * However reading entries from a log file is nearly instantaneous, therefore it does not
     * matter than stop() does not interrupt collectLogEntries() immediately.
     */
}
}
}
}
