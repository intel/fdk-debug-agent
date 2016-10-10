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
#include <cAVS/LogStreamer.hpp>
#include <System/IfdkStreamHeader.hpp>
#include <TestCommon/TestHelpers.hpp>
#include "catch.hpp"
#include <ostream>
#include <sstream>
#include <string>

using namespace debug_agent::cavs;
using namespace debug_agent::system;

/* Current IFDK:cavs:log generic IFDK header content */
static const std::string systemType = "cavs";
static const std::string formatType = "fwlogs";
static const int majorVersion = 1;
static const int minorVersion = 0;

class TestLoggerMock : public Logger
{
public:
    TestLoggerMock(size_t nbBlocks)
        : mNbBlocks(nbBlocks), mBlockNumber(0), mExpectedBlocksStream(), mMockedParameter()
    {
        /* Write the number of module entries, and the module entry table.
        *  In this test, there is no module entry table, but we still have to write the
        *  number of module entries, i.e. 0 */
        static const int nbModuleEntriesBytesLength = 4;
        char nbModuleEntriesBytes[nbModuleEntriesBytesLength] = {0x00, 0x00, 0x00, 0x00};
        mExpectedBlocksStream.write(nbModuleEntriesBytes, nbModuleEntriesBytesLength);

        /* Write the log blocks */
        for (size_t i = 0; i < mNbBlocks; ++i) {
            std::unique_ptr<LogBlock> block(std::move(generateLogBlock(i)));
            mExpectedBlocksStream << *block;
        }
    }

    const std::stringstream &getExpectedBlocksStream() { return mExpectedBlocksStream; }

    virtual void setParameters(const Parameters &parameters) override
    {
        mMockedParameter = parameters;
    }

    virtual Logger::Parameters getParameters() override { return mMockedParameter; }

    virtual std::unique_ptr<LogBlock> readLogBlock() override
    {
        if (mBlockNumber < mNbBlocks) {

            std::unique_ptr<LogBlock> block(std::move(generateLogBlock(mBlockNumber++)));
            return block;
        } else {
            /* Emulate an error using Logger::Exception after the expected number of iterations to
             * stop the log stream. */
            throw Logger::Exception("No more log");
        }
    }

    virtual void stop() noexcept override {}

    static std::unique_ptr<LogBlock> generateLogBlock(size_t blockNumber)
    {
        const unsigned int coreId = 0;
        const std::string logData("Test log block number " + std::to_string(blockNumber));

        std::unique_ptr<LogBlock> block = std::make_unique<LogBlock>(coreId, logData.size());
        std::copy(logData.begin(), logData.end(), block->getLogData().begin());

        return block;
    }

    const size_t mNbBlocks;
    size_t mBlockNumber;
    std::stringstream mExpectedBlocksStream;
    Parameters mMockedParameter;
};

void initFakeModuleEntries(std::vector<dsp_fw::ModuleEntry> &moduleEntries, size_t nbEntries)
{
    for (size_t i = 0; i < nbEntries; ++i) {

        dsp_fw::ModuleEntry entry;
        entry.module_id = static_cast<uint32_t>(i);
        /* Fill UUID with current entry value shifted by uuid word index to get a pattern like:
         * uuid[0] = 0x000000<i>
         * uuid[1] = 0x0000<i>00
         * uuid[2] = 0x00<i>0000
         * uuid[3] = 0x<i>000000
         * Which will be binary streamed out:
         * uuid[0] = 0x<i>000000
         * uuid[1] = 0x00<i>0000
         * uuid[2] = 0x0000<i>00
         * uuid[3] = 0x000000<i>
         */
        entry.uuid[0] = static_cast<uint32_t>(i);
        entry.uuid[1] = static_cast<uint32_t>(i) << 8;
        entry.uuid[2] = static_cast<uint32_t>(i) << 16;
        entry.uuid[3] = static_cast<uint32_t>(i) << 24;
        moduleEntries.push_back(entry);
    }
}

TEST_CASE("Test IFDK cAVS Log stream", "[stream]")
{
    // Create a fake module entries table with no entries
    std::vector<dsp_fw::ModuleEntry> moduleEntries;

    // Create a LogStreamer to be tested, using a fake TestLoggerMock as Logger
    TestLoggerMock fakeLogger(50);
    LogStreamer logStreamer(fakeLogger, moduleEntries);

    std::stringstream outStream;
    std::stringstream expectedOutStream;

    // Compute expected stream
    const IfdkStreamHeader logIfdkHeader(systemType, formatType, majorVersion, minorVersion);
    expectedOutStream << logIfdkHeader;
    expectedOutStream << fakeLogger.getExpectedBlocksStream().str();

    CHECK_THROWS_AS_MSG(outStream << logStreamer, Streamer::Exception,
                        "Fail to read log: No more log");

    CHECK(outStream.str() == expectedOutStream.str());
}

TEST_CASE("Test module entries to stream", "[streaming]")
{
    // Create a fake module entries table
    std::vector<dsp_fw::ModuleEntry> moduleEntries;
    initFakeModuleEntries(moduleEntries, 3);

    // Create a LogStreamer to be tested, using a fake TestLoggerMock as Logger which will not
    // generate any log block
    TestLoggerMock fakeLogger(0);
    LogStreamer logStreamer(fakeLogger, moduleEntries);

    std::stringstream outStream;
    CHECK_THROWS_AS_MSG(outStream << logStreamer, Streamer::Exception,
                        "Fail to read log: No more log");

    /* Expected stream size:
     * - 4 bytes for Module ID per entry
     * - 16 bytes for UUID per entry
     * - 8 bytes for versions per entry
     * - 3 entries in test table
     */
    static const size_t expectedOutModuleEntriesStreamLength = 4 + (4 + 16 + 8) * 3;
    /* Expected stream: versions are currently all 0 and UUID is filled by initFakeModuleEntries().
     */
    char expectedOutModuleEntriesStreamBytes[expectedOutModuleEntriesStreamLength] = {
        // Number of Module Entries (little endian)
        0x03, 0x00, 0x00, 0x00,
        // Entry 1
        // Module ID (little endian)
        0x00, 0x00, 0x00, 0x00,
        // UUID
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00,
        // Versions (little endian)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Entry 2
        // Module ID (little endian)
        0x01, 0x00, 0x00, 0x00,
        // UUID
        0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
        0x01,
        // Versions (little endian)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Entry 3
        // Module ID (little endian)
        0x02, 0x00, 0x00, 0x00,
        // UUID
        0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
        0x02,
        // Versions (little endian)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    // Expected stream: IFDK header + expectedOutModuleEntriesStreamBytes
    const IfdkStreamHeader logIfdkHeader(systemType, formatType, majorVersion, minorVersion);
    std::stringstream expectedOutStream;
    expectedOutStream << logIfdkHeader;
    expectedOutStream.write(expectedOutModuleEntriesStreamBytes,
                            expectedOutModuleEntriesStreamLength);

    CHECK(outStream.str() == expectedOutStream.str());
}
