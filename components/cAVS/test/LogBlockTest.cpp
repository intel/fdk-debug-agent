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
#include <cAVS/LogBlock.hpp>
#include <TestCommon/TestHelpers.hpp>
#include <catch.hpp>
#include <string>
#include <sstream>
#include <algorithm>

using namespace debug_agent::cavs;

TEST_CASE("Core ID validity", "[constructor]")
{
    static const unsigned int minCoreId = 0;
    static const unsigned int maxCoreId = 15;
    static const unsigned int overMaxCoreId = 16;

    CHECK_THROWS_MSG(LogBlock b(overMaxCoreId),
        "Invalid Core ID: " + std::to_string(overMaxCoreId) + " should be in [0..15]");

    CHECK_NOTHROW(LogBlock b(minCoreId));

    CHECK_NOTHROW(LogBlock b(maxCoreId));
}

TEST_CASE("Size validity", "[streaming]")
{
    static const int coreId = 15;
    static const unsigned int maxBlockSize = 1234;
    static const unsigned int overmaxBlocSize = maxBlockSize + 1;

    LogBlockBase<maxBlockSize> block(coreId, overmaxBlocSize);

    std::stringstream blockStream;
    CHECK_THROWS_MSG(blockStream << block,
        "Log block size exceeds maximum value");
}

TEST_CASE("Stream empty LogBlock", "[streaming]")
{
    static const int coreId = 15;

    LogBlock block(coreId, 0);

    std::stringstream blockStream;
    blockStream << block;

    /* Expected block header: 32bits word (little endian) :
     * - bits [0..27]: block data length (0)
     * - bits [28..31]: block log core ID (coreId)
     */
    static const std::size_t blockHeaderStreamLength = 4;
    unsigned char expectedBlockStreamBytes[blockHeaderStreamLength] = {0, 0, 0, coreId << 4};

    std::string expectedBlockStream(reinterpret_cast<char *>(expectedBlockStreamBytes),
        blockHeaderStreamLength);

    CHECK(blockStream.str() == expectedBlockStream);
}

TEST_CASE("Stream LogBlock", "[streaming]")
{
    static const int coreId = 15;
    static const std::string aBeautifulLog(
        "/*"
        "********************************************************************************"
        "*                              INTEL CONFIDENTIAL"
        "*   Copyright(C) 2015 Intel Corporation. All Rights Reserved."
        "*   The source code contained  or  described herein and all documents related to"
        "*   the source code (\"Material\") are owned by Intel Corporation or its suppliers"
        "*   or licensors.  Title to the  Material remains with  Intel Corporation or its"
        "*   suppliers and licensors. The Material contains trade secrets and proprietary"
        "*   and  confidential  information of  Intel or its suppliers and licensors. The"
        "*   Material  is  protected  by  worldwide  copyright  and trade secret laws and"
        "*   treaty  provisions. No part of the Material may be used, copied, reproduced,"
        "*   modified, published, uploaded, posted, transmitted, distributed or disclosed"
        "*   in any way without Intel's prior express written permission."
        "*   No license  under any  patent, copyright, trade secret or other intellectual"
        "*   property right is granted to or conferred upon you by disclosure or delivery"
        "*   of the Materials,  either expressly, by implication, inducement, estoppel or"
        "*   otherwise.  Any  license  under  such  intellectual property  rights must be"
        "*   express and approved by Intel in writing."
        "*"
        "********************************************************************************"
        "*/"
    );
    /* Create the log block */
    LogBlock block(coreId, aBeautifulLog.size());
    /* Fill log block log data vector with aBeautifulLog */
    std::copy(aBeautifulLog.begin(), aBeautifulLog.end(), block.getLogData().begin());
    /* Stream out the log block */
    std::stringstream blockStream;
    blockStream << block;

    /* Expected block header: 32bits word (little endian) :
     * - bits [0..27]: block data length (aBeautifulLog.size())
     * - bits [28..31]: block log core ID (coreId)
     */
    static const std::size_t blockHeaderStreamLength = 4;
    unsigned char blockHeaderStream[blockHeaderStreamLength] = {
        static_cast<unsigned char>(aBeautifulLog.size() & 0xFF), /* First byte: least significant */
        static_cast<unsigned char>((aBeautifulLog.size() >> 8) & 0xFF),
        0, /* This unit test is designed for a aBeautifulLog.size() < 65536 */
        coreId << 4};
    std::string expectedBlockStreamHeader(reinterpret_cast<char *>(blockHeaderStream),
        blockHeaderStreamLength);

    /* Expected block stream is the concatenation of the expected block header and the expected
     * block log data */
    std::stringstream expectedBlockStream;
    expectedBlockStream << expectedBlockStreamHeader << aBeautifulLog;

    CHECK(blockStream.str() == expectedBlockStream.str());
}
