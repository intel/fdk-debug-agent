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

class TestLoggerMock: public Logger
{
public:
    TestLoggerMock(size_t nbBlocks):
        mNbBlocks(nbBlocks),
        mBlockNumber(0),
        mExpectedBlocksStream(),
        mMockedParameter()
    {
        for (size_t i = 0; i < mNbBlocks; ++i) {

            std::unique_ptr<LogBlock> block(std::move(generateLogBlock(i)));
            mExpectedBlocksStream << *block;
        }
    }

    const std::stringstream &getExpectedBlocksStream()
    {
        return mExpectedBlocksStream;
    }

    virtual void setParameters(const Parameters &parameters) override
    {
        mMockedParameter = parameters;
    }

    virtual Logger::Parameters getParameters() override
    {
        return mMockedParameter;
    }

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

    virtual void stop() NOEXCEPT override
    {
    }

    static std::unique_ptr<LogBlock> generateLogBlock(size_t blockNumber)
    {
        const unsigned int coreId = 0;
        const std::string logData("Test log block number " + std::to_string(blockNumber));

        std::unique_ptr<LogBlock> block(new LogBlock(coreId, logData.size()));
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

    CHECK_THROWS_MSG(outStream << logStreamer,
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
    CHECK_THROWS_MSG(outStream << logStreamer,
        "Fail to read log: No more log");

    /* Expected stream size:
     * - 4 bytes for Module ID per entry
     * - 16 bytes for UUID per entry
     * - 8 bytes for versions per entry
     * - 3 entries in test table
     */
    static const size_t expectedOutModuleEntriesStreamLength = (4 + 16 + 8) * 3;
    /* Expected stream: versions are currently all 0 and UUID is filled by initFakeModuleEntries().
     */
    char expectedOutModuleEntriesStreamBytes[expectedOutModuleEntriesStreamLength] = {
        // Entry 1
            // Module ID (little endian)
            0x00, 0x00, 0x00, 0x00,
            // UUID
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            // Versions (little endian)
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
        // Entry 2
            // Module ID (little endian)
            0x01, 0x00, 0x00, 0x00,
            // UUID
            0x01, 0x00, 0x00, 0x00,
            0x00, 0x01, 0x00, 0x00,
            0x00, 0x00, 0x01, 0x00,
            0x00, 0x00, 0x00, 0x01,
            // Versions (little endian)
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
        // Entry 3
            // Module ID (little endian)
            0x02, 0x00, 0x00, 0x00,
            // UUID
            0x02, 0x00, 0x00, 0x00,
            0x00, 0x02, 0x00, 0x00,
            0x00, 0x00, 0x02, 0x00,
            0x00, 0x00, 0x00, 0x02,
            // Versions (little endian)
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
    };

    // Expected stream: IFDK header + expectedOutModuleEntriesStreamBytes
    const IfdkStreamHeader logIfdkHeader(systemType, formatType, majorVersion, minorVersion);
    std::stringstream expectedOutStream;
    expectedOutStream << logIfdkHeader;
    expectedOutStream.write(expectedOutModuleEntriesStreamBytes,
                            expectedOutModuleEntriesStreamLength);

    CHECK(outStream.str() == expectedOutStream.str());
}