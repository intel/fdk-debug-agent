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
#include "TlvTestLanguage.hpp"
#include "Tlv/TlvUnpack.hpp"
#include "TestCommon/TestHelpers.hpp"
#include "catch.hpp"

TEST_CASE("TlvUnpack", "[ReadBuffer]")
{
    TlvTestLanguage testTlvLanguage;

    SECTION("Null pointer") {

        CHECK_THROWS_MSG(TlvUnpack unpacker(testTlvLanguage, nullptr, 0), "Null pointer");
    }

    SECTION("Read from empty buffer") {

        char buffer[1];
        TlvUnpack unpacker(testTlvLanguage, buffer, 0);

        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == false);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Read 1 TLV from buffer") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0x1234, 0x5678};
        uint32_t tag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t length = static_cast<uint32_t>(sizeof(HelloValueType));

        const size_t tlvListBufferSize = sizeof(tag) + sizeof(length) + sizeof(HelloValueType);
        char tlvListBuffer[tlvListBufferSize];

        *reinterpret_cast<uint32_t *>(&tlvListBuffer[0]) = tag;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[sizeof(tag)]) = length;
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[sizeof(tag) + sizeof(length)]) =
            helloValue;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Read from buffer including invalid tag") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0x1234, 0x5678};
        uint32_t tag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t length = static_cast<uint32_t>(sizeof(HelloValueType));

        const size_t tlvListBufferSize = sizeof(tag) + sizeof(length) + sizeof(HelloValueType);
        char tlvListBuffer[tlvListBufferSize];

        // Let's change the HELLO tag by 0xFFFFFFFF for fun!
        uint32_t badTag = 0xFFFFFFFF;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[0]) = badTag;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[sizeof(tag)]) = length;
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[sizeof(tag) + sizeof(length)]) =
            helloValue;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK_THROWS_MSG(unpacker.readNext(), "Cannot parse unknown tag " + std::to_string(badTag));
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == false);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Read 2 TLV from buffer") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0xDEAD, 0xBEEF};
        uint32_t hTag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t hLength = static_cast<uint32_t>(sizeof(HelloValueType));
        WorldValueType worldValue {0, 42, 1268469841515, 687684186186};
        uint32_t wTag = static_cast<uint32_t>(TlvTestLanguage::Tags::World);
        uint32_t wLength = static_cast<uint32_t>(sizeof(WorldValueType));


        const size_t tlvListBufferSize =
            sizeof(hTag) + sizeof(hLength) + sizeof(HelloValueType) +
            sizeof(wTag) + sizeof(wLength) + sizeof(WorldValueType);
        char tlvListBuffer[tlvListBufferSize];

        size_t index = 0;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hTag;
        index += sizeof(hTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hLength;
        index += sizeof(hLength);
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[index]) = helloValue;
        index += sizeof(helloValue);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wTag;
        index += sizeof(wTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wLength;
        index += sizeof(wLength);
        *reinterpret_cast<WorldValueType *>(&tlvListBuffer[index]) = worldValue;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == true);
        CHECK(testTlvLanguage.world == worldValue);
    }

    SECTION("Read 3 TLV from buffer, including an array one") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0xDEAD, 0xBEEF};
        uint32_t hTag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t hLength = static_cast<uint32_t>(sizeof(HelloValueType));

        const size_t nbThe = 5;
        std::vector<TheValueType> theValues { {1}, {2}, {3}, {4}, {5} };
        uint32_t tTag = static_cast<uint32_t>(TlvTestLanguage::Tags::The);
        uint32_t tLength = static_cast<uint32_t>(sizeof(TheValueType) * theValues.size());
        REQUIRE(theValues.size() == nbThe);

        WorldValueType worldValue {0, 42, 1268469841515, 687684186186};
        uint32_t wTag = static_cast<uint32_t>(TlvTestLanguage::Tags::World);
        uint32_t wLength = static_cast<uint32_t>(sizeof(WorldValueType));


        const size_t tlvListBufferSize =
            sizeof(hTag) + sizeof(hLength) + sizeof(HelloValueType) +
            sizeof(tTag) + sizeof(tLength) + sizeof(TheValueType) * nbThe +
            sizeof(wTag) + sizeof(wLength) + sizeof(WorldValueType);
        char tlvListBuffer[tlvListBufferSize];

        size_t index = 0;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hTag;
        index += sizeof(hTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hLength;
        index += sizeof(hLength);
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[index]) = helloValue;
        index += sizeof(helloValue);

        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = tTag;
        index += sizeof(tTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = tLength;
        index += sizeof(tLength);
        for (auto &aTheValue : theValues) {

            *reinterpret_cast<TheValueType *>(&tlvListBuffer[index]) = aTheValue;
            index += sizeof(aTheValue);
        }

        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wTag;
        index += sizeof(wTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wLength;
        index += sizeof(wLength);
        *reinterpret_cast<WorldValueType *>(&tlvListBuffer[index]) = worldValue;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == theValues.size());
        CHECK(testTlvLanguage.the == theValues);
        CHECK(testTlvLanguage.isWorldValid == true);
        CHECK(testTlvLanguage.world == worldValue);
    }

    SECTION("Read 3 TLV from buffer, including multiple 0 sizes") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0xDEAD, 0xBEEF};
        uint32_t hTag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t hLength = static_cast<uint32_t>(sizeof(HelloValueType));

        uint32_t tTag = static_cast<uint32_t>(TlvTestLanguage::Tags::The);
        uint32_t tLength = 0;

        uint32_t wTag = static_cast<uint32_t>(TlvTestLanguage::Tags::World);
        uint32_t wLength = 0;


        const size_t tlvListBufferSize =
            sizeof(hTag) + sizeof(hLength) + sizeof(HelloValueType) +
            sizeof(tTag) + sizeof(tLength) +
            sizeof(wTag) + sizeof(wLength);
        char tlvListBuffer[tlvListBufferSize];

        size_t index = 0;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hTag;
        index += sizeof(hTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hLength;
        index += sizeof(hLength);
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[index]) = helloValue;
        index += sizeof(helloValue);

        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = tTag;
        index += sizeof(tTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = tLength;
        index += sizeof(tLength);

        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wTag;
        index += sizeof(wTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wLength;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK(unpacker.readNext() == true);
        CHECK_THROWS_MSG(unpacker.readNext(), "Error reading value for tag "
            + std::to_string(tTag) + " (size " + std::to_string(tLength)
            + " bytes): Invalid binary size for TLV value read");
        CHECK_THROWS_MSG(unpacker.readNext(), "Error reading value for tag "
            + std::to_string(wTag) + " (size " + std::to_string(wLength)
            + " bytes): Invalid binary size for TLV value read");

        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Read from buffer including bad size as last element") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0xDEAD, 0xBEEF};
        uint32_t hTag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t hLength = static_cast<uint32_t>(sizeof(HelloValueType));
        WorldValueType worldValue {0, 42, 1268469841515, 687684186186};
        uint32_t wTag = static_cast<uint32_t>(TlvTestLanguage::Tags::World);
        uint32_t wLength = static_cast<uint32_t>(sizeof(WorldValueType));

        // Let's change the WOLRD tag length value for fun!
        wLength += 3;

        const size_t tlvListBufferSize =
            sizeof(hTag) + sizeof(hLength) + sizeof(HelloValueType) +
            sizeof(wTag) + sizeof(wLength) + sizeof(WorldValueType);
        char tlvListBuffer[tlvListBufferSize];

        size_t index = 0;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hTag;
        index += sizeof(hTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hLength;
        index += sizeof(hLength);
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[index]) = helloValue;
        index += sizeof(helloValue);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wTag;
        index += sizeof(wTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wLength;
        index += sizeof(wLength);
        *reinterpret_cast<WorldValueType *>(&tlvListBuffer[index]) = worldValue;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK(unpacker.readNext() == true);
        CHECK_THROWS_MSG(unpacker.readNext(), "Incomplete value for tag " + std::to_string(wTag));
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Read from buffer including bad size as middle element") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0xDEAD, 0xBEEF};
        uint32_t hTag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t hLength = static_cast<uint32_t>(sizeof(HelloValueType));
        WorldValueType worldValue {0, 42, 1268469841515, 687684186186};
        uint32_t wTag = static_cast<uint32_t>(TlvTestLanguage::Tags::World);
        uint32_t wLength = static_cast<uint32_t>(sizeof(WorldValueType));

        // Let's change the HELLO tag length value for fun!
        hLength += 3;

        const size_t tlvListBufferSize =
            sizeof(hTag) + sizeof(hLength) + sizeof(HelloValueType) +
            sizeof(wTag) + sizeof(wLength) + sizeof(WorldValueType);
        char tlvListBuffer[tlvListBufferSize];

        size_t index = 0;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hTag;
        index += sizeof(hTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hLength;
        index += sizeof(hLength);
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[index]) = helloValue;
        index += sizeof(helloValue);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wTag;
        index += sizeof(wTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wLength;
        index += sizeof(wLength);
        *reinterpret_cast<WorldValueType *>(&tlvListBuffer[index]) = worldValue;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK_THROWS_MSG(unpacker.readNext(), "Error reading value for tag "
            + std::to_string(hTag) + " (size " + std::to_string(hLength)
            + " bytes): Invalid binary size for TLV value read");
        // Obviously because of the invalid length, the next read will fail because of the random
        // tag value:
        CHECK_THROWS(unpacker.readNext());
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == false);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Read from buffer with last TLV incomplete: missing value part") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0xDEAD, 0xBEEF};
        uint32_t hTag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t hLength = static_cast<uint32_t>(sizeof(HelloValueType));
        WorldValueType worldValue {0, 42, 1268469841515, 687684186186};
        uint32_t wTag = static_cast<uint32_t>(TlvTestLanguage::Tags::World);
        uint32_t wLength = static_cast<uint32_t>(sizeof(WorldValueType));

        // Let's remove the WORLD tag value for fun!
        const size_t tlvListBufferSize =
            sizeof(hTag) + sizeof(hLength) + sizeof(HelloValueType) +
            sizeof(wTag) + sizeof(wLength) /*+ sizeof(WorldValueType)*/;
        char tlvListBuffer[tlvListBufferSize];

        size_t index = 0;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hTag;
        index += sizeof(hTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hLength;
        index += sizeof(hLength);
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[index]) = helloValue;
        index += sizeof(helloValue);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wTag;
        index += sizeof(wTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wLength;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK(unpacker.readNext() == true);
        CHECK_THROWS_MSG(unpacker.readNext(), "Incomplete value for tag " + std::to_string(wTag));
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Read from buffer with last TLV incomplete: missing value and length") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0xDEAD, 0xBEEF};
        uint32_t hTag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t hLength = static_cast<uint32_t>(sizeof(HelloValueType));
        WorldValueType worldValue {0, 42, 1268469841515, 687684186186};
        uint32_t wTag = static_cast<uint32_t>(TlvTestLanguage::Tags::World);
        uint32_t wLength = static_cast<uint32_t>(sizeof(WorldValueType));

        // Let's remove the WORLD tag value and length for fun!
        const size_t tlvListBufferSize =
            sizeof(hTag) + sizeof(hLength) + sizeof(HelloValueType) +
            sizeof(wTag) /*+ sizeof(wLength) + sizeof(WorldValueType)*/;
        char tlvListBuffer[tlvListBufferSize];

        size_t index = 0;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hTag;
        index += sizeof(hTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hLength;
        index += sizeof(hLength);
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[index]) = helloValue;
        index += sizeof(helloValue);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wTag;

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize);

        CHECK(unpacker.readNext() == true);
        CHECK_THROWS_MSG(unpacker.readNext(), "Incomplete TLV at end of buffer");
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Read from buffer with last TLV incomplete: missing tag part and value and length") {

        // Construct a test TLV list buffer
        HelloValueType helloValue {0xDEAD, 0xBEEF};
        uint32_t hTag = static_cast<uint32_t>(TlvTestLanguage::Tags::Hello);
        uint32_t hLength = static_cast<uint32_t>(sizeof(HelloValueType));
        WorldValueType worldValue {0, 42, 1268469841515, 687684186186};
        uint32_t wTag = static_cast<uint32_t>(TlvTestLanguage::Tags::World);
        uint32_t wLength = static_cast<uint32_t>(sizeof(WorldValueType));

        // Let's remove the WORLD tag value and length for fun!
        const size_t tlvListBufferSize =
            sizeof(hTag) + sizeof(hLength) + sizeof(HelloValueType) +
            sizeof(wTag) /* + sizeof(wLength) + sizeof(WorldValueType)*/;
        char tlvListBuffer[tlvListBufferSize];

        size_t index = 0;
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hTag;
        index += sizeof(hTag);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = hLength;
        index += sizeof(hLength);
        *reinterpret_cast<HelloValueType *>(&tlvListBuffer[index]) = helloValue;
        index += sizeof(helloValue);
        *reinterpret_cast<uint32_t *>(&tlvListBuffer[index]) = wTag;

        // Remove 2 bytes in buffer size: tag WORLD will be incomplete
        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, tlvListBuffer, tlvListBufferSize - 2);

        CHECK(unpacker.readNext() == true);
        CHECK_THROWS_MSG(unpacker.readNext(), "Incomplete TLV at end of buffer");
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }
}