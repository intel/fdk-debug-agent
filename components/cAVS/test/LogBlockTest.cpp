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

    CHECK_THROWS_AS_MSG(LogBlock b(overMaxCoreId), LogBlock::Exception,
                        "Invalid Core ID: " + std::to_string(overMaxCoreId) +
                            " should be in [0..15]");

    CHECK_NOTHROW(LogBlock b(minCoreId));

    CHECK_NOTHROW(LogBlock b(maxCoreId));
}

TEST_CASE("Size validity", "[streaming]")
{
    static const int coreId = 15;
    static const unsigned int maxBlockSize = 1234;
    static const unsigned int overmaxBlocSize = maxBlockSize + 1;

    using TestedLogBlocType = LogBlockBase<maxBlockSize>;

    TestedLogBlocType block(coreId, overmaxBlocSize);

    std::stringstream blockStream;
    CHECK_THROWS_AS_MSG(blockStream << block, TestedLogBlocType::Exception,
                        "Log block size exceeds maximum value");
}

TEST_CASE("cAVS block size validity", "[streaming]")
{
    static const int coreId = 15;
    static const unsigned int overmaxBlocSize = cavsLogBlockMaxSize + 1;

    LogBlock block(coreId, overmaxBlocSize);

    std::stringstream blockStream;
    CHECK_THROWS_AS_MSG(blockStream << block, LogBlock::Exception,
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
        "* Copyright (c) 2015, Intel Corporation"
        "* All rights reserved."
        "*"
        "* Redistribution and use in source and binary forms, with or without modification,"
        "* are permitted provided that the following conditions are met:"
        "*"
        "* 1. Redistributions of source code must retain the above copyright notice, this"
        "* list of conditions and the following disclaimer."
        "*"
        "* 2. Redistributions in binary form must reproduce the above copyright notice,"
        "* this list of conditions and the following disclaimer in the documentation and/or"
        "* other materials provided with the distribution."
        "*"
        "* 3. Neither the name of the copyright holder nor the names of its contributors"
        "* may be used to endorse or promote products derived from this software without"
        "* specific prior written permission."
        "*"
        "* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND"
        "* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED"
        "* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE"
        "* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR"
        "* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES"
        "* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;"
        "* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON"
        "* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT"
        "* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS"
        "* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
        "*/");
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
