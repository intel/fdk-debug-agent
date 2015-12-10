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