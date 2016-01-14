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
#include "Util/StructureChangeTracking.hpp"
#include <catch.hpp>

using namespace debug_agent::util;

TEST_CASE("StructureChangeTracking")
{
    /* Note: this test cannot fail:
     * - either it does not compile
     * - or it is successful
     * This is the wished behaviour.
     */

    struct StructA
    {
        uint16_t prop1;
        uint16_t prop2;
    };

    CHECK_SIZE(StructA, 4);
    CHECK_MEMBER(StructA, prop1, 0, uint16_t);
    CHECK_MEMBER(StructA, prop2, 2, uint16_t);

    struct StructB
    {
        uint32_t prop1;
        StructA prop2;
        uint32_t list1Size;
        uint32_t list2Size;
        uint32_t list1[1];
        uint32_t list2[1];
    };

    CHECK_SIZE(StructB, 24);
    CHECK_MEMBER(StructB, prop1, 0, uint32_t);
    CHECK_MEMBER(StructB, prop2, 4, StructA);
    CHECK_MEMBER(StructB, list1Size, 8, uint32_t);
    CHECK_MEMBER(StructB, list2Size, 12, uint32_t);
    CHECK_MEMBER(StructB, list1, 16, uint32_t[1]);
    CHECK_MEMBER(StructB, list2, 20, uint32_t[1]);
}
