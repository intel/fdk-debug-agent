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

#include "cAVS/Windows/RealTimeWppClient.hpp"
#include "cAVS/Windows/FileWppClient.hpp"
#include "TestCommon/ThreadHelpers.hpp"
#include <catch.hpp>
#include <future>

using namespace debug_agent::cavs::windows;
using namespace debug_agent::test_common;

/** Unfortunately these tests don't collect firmware log entries because it's not possible on host.
 */

static const std::string LogFileName = PROJECT_PATH "Windows/data/cAVS/fw_log_38_entries.etl";
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

    std::size_t getEntryCount() { return mEntryCount; }

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
    ThreadHelpers::ensureNonBlocking([&]() { client.collectLogEntries(listener); });
}

TEST_CASE("RealTimeWppClient: stopping while collecting", "[admin_rights]")
{
    RealTimeWppClient client;

    /** Will stop the client after 200 ms, while collectLogEntries()
     * is running */
    std::future<void> future = std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        client.stop();
    });

    /** Starting collection, it should be blocked during approximately 200 ms. If it takes more
     * than 5000ms, the ThreadHelpers::ensureNonBlocking() will fail. */
    CounterListener listener;
    ThreadHelpers::ensureNonBlocking([&]() { client.collectLogEntries(listener); });

    future.get();
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

TEST_CASE("FileWppClient: counting entries", "[admin_rights]")
{
    FileWppClient client(LogFileName);

    /* Collecting */
    CounterListener listener;
    client.collectLogEntries(listener);

    /* Checking entry count */
    CHECK(listener.getEntryCount() == LogFileEntryCount);
}
