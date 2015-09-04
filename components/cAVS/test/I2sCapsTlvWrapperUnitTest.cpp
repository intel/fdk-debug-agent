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
#include "cAVS/HwConfig.hpp"
#include "TestCommon/TestHelpers.hpp"
#include "catch.hpp"

using namespace debug_agent::cavs;

static const size_t minimumI2sCapsValueSize = 8;
#define i2sCapsValueSize(controllerCount) \
    (minimumI2sCapsValueSize + ((controllerCount) * sizeof(uint32_t)))

TEST_CASE("I2sCapsTlvWrapper", "[WrapperRead]")
{
    HwConfig::I2sCaps testValue;
    bool testValueIsValid = true;

    HwConfig::I2sCapsTlvWrapper i2sCapsTlvWrapper(testValue, testValueIsValid);
    CHECK(testValueIsValid == false);

    // size check ok
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(0)) == true);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(1)) == true);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(2)) == true);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(3)) == true);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(5)) == true);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(13)) == true);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(55)) == true);
    // size check ko
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(0) - 1) == false);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(0) + 1) == false);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(5) - 1) == false);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(5) + 1) == false);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(5) - 2) == false);
    CHECK(i2sCapsTlvWrapper.isValidSize(i2sCapsValueSize(5) + 3) == false);

    // Read invalid size
    const size_t invalidSize = i2sCapsValueSize(0) + 1;
    char invalidSizeTlv[invalidSize];
    CHECK_THROWS_MSG(i2sCapsTlvWrapper.readFrom(invalidSizeTlv, invalidSize),
                     "Invalid binary size ("
                     + std::to_string(invalidSize)
                     + " bytes) for a TLV I2sCaps value");
    CHECK(testValueIsValid == false);

    // Read inconsistent value (controller_count to high)
    const size_t inconstistentValueSize = i2sCapsValueSize(0);
    char inconsistentSizeTlv[inconstistentValueSize] {
            // version
            0x00, 0x00, 0x00, 0x00,
            // controller_count = 2 instead of 0
            0x02, 0x00, 0x00, 0x00
    };
    CHECK_THROWS_MSG(i2sCapsTlvWrapper.readFrom(inconsistentSizeTlv, inconstistentValueSize),
                     "struct I2sCapabilities inconsistency");
    CHECK(testValueIsValid == false);

    // Read inconsistent value (controller_count to low)
    const size_t inconstistent2ValueSize = i2sCapsValueSize(1);
    char inconsistentSizeTlv2[inconstistent2ValueSize] {
            // version
            0x00, 0x00, 0x00, 0x00,
            // controller_count = 0 instead of 1
            0x00, 0x00, 0x00, 0x00,
            // First and last controller_base_addr
            0x00, 0x00, 0x00, 0x00
    };
    CHECK_THROWS_MSG(i2sCapsTlvWrapper.readFrom(inconsistentSizeTlv, inconstistent2ValueSize),
                     "struct I2sCapabilities inconsistency");
    CHECK(testValueIsValid == false);

    // Read empty controller_base_addr
    const size_t emptyControllerBaseAddrSize = i2sCapsValueSize(0);
    char emptyControllerBaseAddr[emptyControllerBaseAddrSize] {
            // version
            0x01, 0x02, 0x03, 0x04,
            // controller_count = 0 instead of 1
            0x00, 0x00, 0x00, 0x00
    };
    CHECK_NOTHROW(i2sCapsTlvWrapper.readFrom(emptyControllerBaseAddr, emptyControllerBaseAddrSize));
    CHECK(testValue.version == *(reinterpret_cast<uint32_t *>(emptyControllerBaseAddr)));
    CHECK(testValue.controllerBaseAddr.size() ==
        *(reinterpret_cast<uint32_t *>(emptyControllerBaseAddr + sizeof(uint32_t))));

    CHECK(testValueIsValid == true);
    i2sCapsTlvWrapper.invalidate();

    // Read 2 elements controller_base_addr
    const size_t twoElementsControllerBaseAddrSize = i2sCapsValueSize(2);
    char twoElementsControllerBaseAddr[twoElementsControllerBaseAddrSize] {
            // version
            0x01, 0x02, 0x03, 0x04,
            // controller_count = 2
            0x02, 0x00, 0x00, 0x00,
            // First controller_base_addr
            0x0A, 0x0B, 0x0C, 0x0D,
            // Last controller_base_addr
            0x0E, 0x0F, 0x09, 0x08
    };
    CHECK_NOTHROW(i2sCapsTlvWrapper.readFrom(
        twoElementsControllerBaseAddr,
        twoElementsControllerBaseAddrSize));
    CHECK(testValue.version == *(reinterpret_cast<uint32_t *>(emptyControllerBaseAddr)));
    CHECK(testValue.controllerBaseAddr.size() ==
        *(reinterpret_cast<uint32_t *>(twoElementsControllerBaseAddr + sizeof(uint32_t))));

    CHECK(testValue.controllerBaseAddr[0] ==
        *(reinterpret_cast<uint32_t *>(twoElementsControllerBaseAddr + 2 * sizeof(uint32_t))));
    CHECK(testValue.controllerBaseAddr[1] ==
        *(reinterpret_cast<uint32_t *>(twoElementsControllerBaseAddr + 3 * sizeof(uint32_t))));

    CHECK(testValueIsValid == true);

    i2sCapsTlvWrapper.invalidate();
    CHECK(testValueIsValid == false);
}