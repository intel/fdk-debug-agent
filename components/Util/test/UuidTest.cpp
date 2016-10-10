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
#include "Util/Uuid.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace debug_agent::util;

TEST_CASE("Uuid: to string")
{
    Uuid uid = {0xDB264037, 0x6BA1, 0x4DC0, {0xAE, 0x16, 0x5C, 0x60, 0xAD, 0x47, 0x0E, 0xDD}};
    CHECK(uid.toString() == "DB264037-6BA1-4DC0-AE16-5C60AD470EDD");
}

TEST_CASE("Uuid: 'from' and 'to' other type")
{
    uint8_t otherUidFormat[16] = {0x37, 0x40, 0x26, 0xDB, 0xA1, 0x6B, 0xC0, 0x4D,
                                  0xAE, 0x16, 0x5C, 0x60, 0xAD, 0x47, 0x0E, 0xDD};

    Uuid uid = {0xDB264037, 0x6BA1, 0x4DC0, {0xAE, 0x16, 0x5C, 0x60, 0xAD, 0x47, 0x0E, 0xDD}};

    /* Conversion other uuid format -> Uuid format */
    Uuid convertedUuid;
    convertedUuid.fromOtherUuidType(otherUidFormat);
    CHECK(uid == convertedUuid);

    /* Conversion Uuid format -> other uuid format */
    uint32_t convertedOtherUidFormat[4];
    uid.toOtherUuidType(convertedOtherUidFormat);
    CHECK(memcmp(otherUidFormat, convertedOtherUidFormat, sizeof(otherUidFormat)) == 0);
}