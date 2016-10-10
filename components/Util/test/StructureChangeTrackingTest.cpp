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
