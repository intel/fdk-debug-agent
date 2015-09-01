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
#include "Util/StringHelper.hpp"
#include <catch.hpp>

using namespace debug_agent::util;

TEST_CASE("StringHelper: getStringFromFixedSizeArray")
{
    static const std::size_t length = 3;

    const char test1[] = { '\0', '\0', '\0' };
    CHECK(StringHelper::getStringFromFixedSizeArray(test1, length) == "");

    const char test2[] = { 'a', '\0', '\0' };
    CHECK(StringHelper::getStringFromFixedSizeArray(test2, length) == "a");

    const char test3[] = { 'a', 'b', '\0' };
    CHECK(StringHelper::getStringFromFixedSizeArray(test3, length) == "ab");

    const char test4[] = { 'a', 'b', 'c' };
    CHECK(StringHelper::getStringFromFixedSizeArray(test4, length) == "abc");

    const char test5[] = { 'e', 'f', 'g' , 'h'};
    CHECK(StringHelper::getStringFromFixedSizeArray(test5, length) == "efg");
}