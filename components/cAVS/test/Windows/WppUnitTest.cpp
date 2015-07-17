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
#include "cAVS/Windows/FileWppClient.hpp"
#include "TestCommon/ThreadHelpers.hpp"
#include <catch.hpp>

using namespace debug_agent::cavs::windows;
using namespace debug_agent::test_common;

/** Unfortunately these tests don't collect firmware log entries because it's not possible on host.
 */

static const char* LogFileName = "data/fw_log_38_entries.etl";
static const std::size_t LogFileEntryCount = 38;

/** This log entry listener counts retrieved entries */
class CounterListener : public WppLogEntryListener
{
public:
    CounterListener() : mEntryCount(0) {}

    virtual void onLogEntry(uint32_t coreId, uint8_t *buffer, uint32_t size) override
    {
        (void)coreId;
        (void)buffer;
        (void)size;
        ++mEntryCount;
    }

    std::size_t getEntryCount()
    {
        return mEntryCount;
    }

private:
    std::size_t mEntryCount;
};

TEST_CASE("RealTimeWppClient: stopping before collecting")
{
    RealTimeWppClient client;

    /** Stopping the client */
    client.stop();

    /** Then calling collect entries: it should returns immediatedly */
    CounterListener listener;
    ThreadHelpers::ensureNonBlocking( [&] () { client.collectLogEntries(listener); } );
}

TEST_CASE("RealTimeWppClient: stopping while collecting")
{
    RealTimeWppClient client;

    /** Starting a thread that will stop the client after 200 ms, while collectLogEntries()
     * is running */
    std::thread th([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        client.stop();
    });

    /** Starting collection, it should be blocked during approximately 200 ms. If it takes more
     * than 5000ms, the ThreadHelpers::ensureNonBlocking() will fail. */
    CounterListener listener;
    ThreadHelpers::ensureNonBlocking([&]() { client.collectLogEntries(listener); });

    th.join();
}

TEST_CASE("FileWppClient: log file not found.")
{
    FileWppClient client("wrong_file");

    CounterListener listener;
    CHECK_THROWS_AS(client.collectLogEntries(listener), WppClient::Exception);
}

TEST_CASE("FileWppClient: stop before collection")
{
    FileWppClient client(LogFileName);

    /* Stopping */
    client.stop();

    /* Then collecting */
    CounterListener listener;
    client.collectLogEntries(listener);

    /* No entry should have been collected. */
    CHECK(listener.getEntryCount() == 0);
}

TEST_CASE("FileWppClient: counting entries")
{
    FileWppClient client(LogFileName);

    /* Collecting */
    CounterListener listener;
    client.collectLogEntries(listener);

    /* Checking entry count */
    CHECK(listener.getEntryCount() == LogFileEntryCount);
}


