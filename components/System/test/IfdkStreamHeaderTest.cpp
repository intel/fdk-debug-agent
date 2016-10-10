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
#include <System/IfdkStreamHeader.hpp>
#include <TestCommon/TestHelpers.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <sstream>

using namespace debug_agent::system;

/**
 * This test checks that IfdkStreamHeader's constructor raises an exception if the
 * format type length exceeds 31 characters
 */
TEST_CASE("Format type string", "[Constructor]")
{
    static const std::string long10charString("1234567890");
    static const std::string long11charString("12345678901");
    static const std::string empty("");
    static const int one = 1;
    static const int ten = 10;

    /* IFDK:1234567890:12345678901[1.1] is 32 characters long */
    CHECK_THROWS_AS_MSG(
        IfdkStreamHeader(long10charString, long11charString, one, one), IfdkStreamHeader::Exception,
        "Format type length exceeds 31 characters for 'IFDK:" + long10charString + ":" +
            long11charString + "[" + std::to_string(one) + "." + std::to_string(one) + "]'");

    /* IFDK:1234567890:1234567890[1.10] is 32 characters long */
    CHECK_THROWS_AS_MSG(
        IfdkStreamHeader(long10charString, long10charString, one, ten), IfdkStreamHeader::Exception,
        "Format type length exceeds 31 characters for 'IFDK:" + long10charString + ":" +
            long10charString + "[" + std::to_string(one) + "." + std::to_string(ten) + "]'");

    /* Invalid System Type */
    CHECK_THROWS_AS_MSG(IfdkStreamHeader(empty, long10charString, 1, 1),
                        IfdkStreamHeader::Exception, "Empty System Type string");

    /* Invalid File Type */
    CHECK_THROWS_AS_MSG(IfdkStreamHeader(long10charString, empty, 1, 1),
                        IfdkStreamHeader::Exception, "Empty File Type string");

    /* IFDK:1234567890:1234567890[1.1] is 31 characters long */
    CHECK_NOTHROW(IfdkStreamHeader(long10charString, long10charString, 1, 1));
}

/**
 * This test checks that IfdkStreamHeader's addProperty() raises an exception if the
 * property key or value length exceeds limits or if a key is added more than once
 */
TEST_CASE("Properties", "[addProperty]")
{
    static const std::string oneCharString("1");
    static const std::string equal("=");
    static const std::string maxString("1234567890123456789012345678901");      // 31 chars long
    static const std::string tooLongString("12345678901234567890123456789012"); // 32 chars long
    static const std::string emptyString("");
    static const std::string systemType("TestSystem");
    static const std::string fileType("TestFile");

    IfdkStreamHeader systemHeader(systemType, fileType, 1, 1);

    /* Too short key is rejected */
    CHECK_THROWS_AS_MSG(systemHeader.addProperty(emptyString, maxString),
                        IfdkStreamHeader::Exception,
                        "Empty property key or value for " + emptyString + equal + maxString);

    /* Too short value is rejected */
    CHECK_THROWS_AS_MSG(systemHeader.addProperty(maxString, emptyString),
                        IfdkStreamHeader::Exception,
                        "Empty property key or value for " + maxString + equal + emptyString);

    /* Too long key is rejected */
    CHECK_THROWS_AS_MSG(systemHeader.addProperty(tooLongString, maxString),
                        IfdkStreamHeader::Exception,
                        "Property length exceeds for " + tooLongString + equal + maxString);

    /* Too long value is rejected */
    CHECK_THROWS_AS_MSG(systemHeader.addProperty(maxString, tooLongString),
                        IfdkStreamHeader::Exception,
                        "Property length exceeds for " + maxString + equal + tooLongString);

    /* Maximum key length and minimum value is accepted */
    CHECK_NOTHROW(systemHeader.addProperty(maxString, oneCharString));

    /* Minimum key length and maximum value is accepted.
     * Subsequent duplication checks are irrelevant if this one fails: REQUIRE instead CHECK.
     */
    REQUIRE_NOTHROW(systemHeader.addProperty(oneCharString, maxString));

    /* Duplication is rejected */
    CHECK_THROWS_AS_MSG(systemHeader.addProperty(oneCharString, maxString),
                        IfdkStreamHeader::Exception,
                        "Property already exists for key '" + oneCharString + "'");

    /* Duplication is still rejected */
    CHECK_THROWS_AS_MSG(systemHeader.addProperty(oneCharString, maxString),
                        IfdkStreamHeader::Exception,
                        "Property already exists for key '" + oneCharString + "'");
}

/**
 * This test checks that IfdkStreamHeader without any property is correctly streamed
 */
TEST_CASE("Stream without property", "[streaming]")
{
    static const std::string systemType("Best System");
    static const std::string fileType("BestFile");
    IfdkStreamHeader systemHeader(systemType, fileType, 1, 1);

    std::stringstream systemHeaderStream;
    systemHeaderStream << systemHeader;

    /*   32 bytes long format type,
     *    4 bytes long number of properties (which is 0) */
    static const std::size_t streamExpectedLength = 32 + 4;
    char expectedSystemHeaderStreamBytes[streamExpectedLength] = {
        'I', 'F', 'D', 'K', ':', 'B', 'e', 's', 't', ' ', 'S', 'y', 's', 't', 'e', 'm', ':', 'B',
        'e', 's', 't', 'F', 'i', 'l', 'e', '[', '1', '.', '1', ']', 0,   0,   0,   0,   0,   0};
    std::string expectedSystemHeaderStream(expectedSystemHeaderStreamBytes, streamExpectedLength);

    CHECK(systemHeaderStream.str() == expectedSystemHeaderStream);
}

/**
 * This test checks that IfdkStreamHeader with 3 properties is correctly streamed
 */
TEST_CASE("Stream with properties", "[streaming]")
{
    static const std::string systemType("Best System");
    static const std::string fileType("BestFile");
    static const std::string propertyKey1("a key");
    static const std::string propertyKey2("a property key");
    static const std::string propertyKey3("a property key of 31 characters");
    static const std::string propertyValue1("hello");
    static const std::string propertyValue2("bonjour");
    static const std::string propertyValue3("a value of 31 characters       ");

    IfdkStreamHeader systemHeader(systemType, fileType, 1, 1);
    systemHeader.addProperty(propertyKey1, propertyValue1);
    systemHeader.addProperty(propertyKey2, propertyValue2);
    systemHeader.addProperty(propertyKey3, propertyValue3);

    std::stringstream systemHeaderStream;
    systemHeaderStream << systemHeader;

    /*     32 bytes long format type,
     *      4 bytes long number of properties (which is 3)
     * 32x2x3 bytes for 3 properties */
    static const std::size_t streamExpectedLength = 32 + 4 + 32 * 2 * 3;
    char expectedSystemHeaderStreamBytes[streamExpectedLength] = {
        'I', 'F', 'D', 'K', ':', 'B', 'e', 's', 't', ' ', 'S', 'y', 's', 't', 'e', 'm', ':', 'B',
        'e', 's', 't', 'F', 'i', 'l', 'e', '[', '1', '.', '1', ']', 0, 0, 3, 0, 0,
        0, /* 0x0003 little endian */
        /* Property 1 key */
        'a', ' ', 'k', 'e', 'y', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        /* Property 1 value */
        'h', 'e', 'l', 'l', 'o', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        /* Property 2 key */
        'a', ' ', 'p', 'r', 'o', 'p', 'e', 'r', 't', 'y', ' ', 'k', 'e', 'y', 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* Property 2 value */
        'b', 'o', 'n', 'j', 'o', 'u', 'r', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        /* Property 3 key */
        'a', ' ', 'p', 'r', 'o', 'p', 'e', 'r', 't', 'y', ' ', 'k', 'e', 'y', ' ', 'o', 'f', ' ',
        '3', '1', ' ', 'c', 'h', 'a', 'r', 'a', 'c', 't', 'e', 'r', 's', 0,
        /* Property 3 value */
        'a', ' ', 'v', 'a', 'l', 'u', 'e', ' ', 'o', 'f', ' ', '3', '1', ' ', 'c', 'h', 'a', 'r',
        'a', 'c', 't', 'e', 'r', 's', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 0};
    std::string expectedSystemHeaderStream(expectedSystemHeaderStreamBytes, streamExpectedLength);

    CHECK(systemHeaderStream.str() == expectedSystemHeaderStream);
}