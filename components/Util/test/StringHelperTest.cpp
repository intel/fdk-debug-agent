/*
 * Copyright (c) 2015-2016, Intel Corporation
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
#include "Util/StringHelper.hpp"
#include <catch.hpp>

using namespace debug_agent::util;

TEST_CASE("StringHelper: getStringFromFixedSizeArray")
{
    static const std::size_t length = 3;

    const char test1[] = {'\0', '\0', '\0'};
    CHECK(StringHelper::getStringFromFixedSizeArray(test1, length) == "");

    const char test2[] = {'a', '\0', '\0'};
    CHECK(StringHelper::getStringFromFixedSizeArray(test2, length) == "a");

    const char test3[] = {'a', 'b', '\0'};
    CHECK(StringHelper::getStringFromFixedSizeArray(test3, length) == "ab");

    const char test4[] = {'a', 'b', 'c'};
    CHECK(StringHelper::getStringFromFixedSizeArray(test4, length) == "abc");

    const char test5[] = {'e', 'f', 'g', 'h'};
    CHECK(StringHelper::getStringFromFixedSizeArray(test5, length) == "efg");
}

TEST_CASE("StringHelper: setStringFromFixedSizeArray")
{
    static const std::size_t length = 3;
    std::vector<char> buf(length);

    StringHelper::setStringToFixedSizeArray(buf.data(), length, std::string(""));
    CHECK(buf == std::vector<char>({'\0', '\0', '\0'}));

    StringHelper::setStringToFixedSizeArray(buf.data(), length, std::string("a"));
    CHECK(buf == std::vector<char>({'a', '\0', '\0'}));

    StringHelper::setStringToFixedSizeArray(buf.data(), length, std::string("ab"));
    CHECK(buf == std::vector<char>({'a', 'b', '\0'}));

    StringHelper::setStringToFixedSizeArray(buf.data(), length, std::string("abc"));
    CHECK(buf == std::vector<char>({'a', 'b', 'c'}));
}

TEST_CASE("StringHelper: trim")
{
    CHECK(StringHelper::trim(" \n ") == "");
    CHECK(StringHelper::trim(" \n abc") == "abc");
    CHECK(StringHelper::trim("abc \n ") == "abc");
    CHECK(StringHelper::trim(" \n abc \n\r ") == "abc");
    CHECK(StringHelper::trim(" \n a b c \n ") == "a b c");
}

TEST_CASE("StringHelper: startsWith")
{
    CHECK(StringHelper::startWith("titi", ""));
    CHECK(StringHelper::startWith("titi", "t"));
    CHECK(StringHelper::startWith("titi", "ti"));
    CHECK(StringHelper::startWith("titi", "tit"));
    CHECK(StringHelper::startWith("titi", "titi"));

    CHECK_FALSE(StringHelper::startWith("titi", "iti"));
    CHECK_FALSE(StringHelper::startWith("titi", "titit"));
}