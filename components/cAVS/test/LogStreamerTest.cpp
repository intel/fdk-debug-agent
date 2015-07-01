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
#include <string>
#include <stdexcept>

using namespace debug_agent::cavs;
using namespace debug_agent::system;

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

TEST_CASE("Test IFDK cAVS Log stream", "[stream]")
{
    /* Current IFDK:cavs:log generic IFDK header content */
    const std::string systemType = "cavs";
    const std::string formatType = "fwlogs";
    const int majorVersion = 1;
    const int minorVersion = 0;
    IfdkStreamHeader logIfdkHeader(systemType,
                                   formatType,
                                   majorVersion,
                                   minorVersion);

    // Create a LogStreamer to be tested, using a fake TestLoggerMock as Logger
    TestLoggerMock fakeLogger(50);
    LogStreamer logStreamer(fakeLogger);

    std::stringstream outStream;
    std::stringstream expectedOutStream;

    // Compute expected stream
    expectedOutStream << logIfdkHeader;
    expectedOutStream << fakeLogger.getExpectedBlocksStream().str();

    CHECK_THROWS_MSG(outStream << logStreamer,
        "Fail to read log: No more log");

    CHECK(outStream.str() == expectedOutStream.str());
}