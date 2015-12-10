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