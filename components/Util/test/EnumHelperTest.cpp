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
#include "Util/EnumHelper.hpp"
#include <catch.hpp>

using namespace debug_agent::util;

enum class MyEnum
{
    val1,
    val2
};

const std::map<MyEnum, std::string> myEnumNames = {{MyEnum::val1, "val1"}, {MyEnum::val2, "val2"}};

TEST_CASE("EnumHelper: isValid")
{
    EnumHelper<MyEnum> helper(myEnumNames);
    CHECK(helper.isValid(MyEnum::val1));
    CHECK(helper.isValid(MyEnum::val2));
    CHECK_FALSE(helper.isValid(static_cast<MyEnum>(5)));
}

TEST_CASE("EnumHelper: toString")
{
    EnumHelper<MyEnum> helper(myEnumNames);
    CHECK(helper.toString(MyEnum::val1) == "val1");
    CHECK(helper.toString(MyEnum::val2) == "val2");

    /* Testing only the beginning of the message because the type name is generated using
     * typeid(EnumType).name(), which is implementation secific (windows and gcc toolchain doesn't
     * produce the same result) .*/
    CHECK(helper.toString(static_cast<MyEnum>(5)).find("Unknown value '5' of") !=
          std::string::npos);
}

TEST_CASE("EnumHelper: fromString")
{
    EnumHelper<MyEnum> helper(myEnumNames);

    MyEnum val;
    CHECK(helper.fromString("val1", val));
    CHECK(val == MyEnum::val1);

    CHECK(helper.fromString("val2", val));
    CHECK(val == MyEnum::val2);

    CHECK_FALSE(helper.fromString("toto", val));
}