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
#include "TlvTestLanguage.hpp"
#include "Tlv/TlvUnpack.hpp"
#include "Util/Buffer.hpp"
#include "TestCommon/TestHelpers.hpp"
#include "catch.hpp"

using namespace debug_agent::util;

TEST_CASE("TlvUnpack", "[ReadBuffer]")
{
    TlvTestLanguage testTlvLanguage;

    SECTION ("Read from empty buffer") {

        Buffer buffer;
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == false);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read 1 TLV from buffer") {

        HelloValueType helloValue{0x1234, 0x5678};

        // Construct a test TLV list buffer
        Buffer buffer{
            54,   0x00, 0x00, 0x00,                        // HelloValueType tag
            8,    0x00, 0x00, 0x00,                        // HelloValueType size
            0x34, 0x12, 0x00, 0x00, 0x78, 0x56, 0x00, 0x00 // HelloValueType value
        };

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read from buffer including invalid tag") {

        // Construct a test TLV list buffer with an invalid tag
        Buffer buffer{
            0xFF, 0xFF, 0xFF, 0xFF,                        // invalid tag
            8,    0x00, 0x00, 0x00,                        // size
            0x34, 0x12, 0x00, 0x00, 0x78, 0x56, 0x00, 0x00 // value
        };
        HelloValueType helloValue{0x1234, 0x5678};

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK_THROWS_AS_MSG(unpacker.readNext(), TlvUnpack::Exception,
                            "Cannot parse unknown tag " + std::to_string(0xFFFFFFFF));
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == false);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read 2 TLV from buffer") {

        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            8,    0x00, 0x00, 0x00,                         // HelloValueType size
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            230,  182,  1,    0x00, // WorldValueType tag
            21,   0x00, 0x00, 0x00, // WorldValueType size
            0,                      // WorldValueType plankRandomGeneratorSeed
            42,   0,    0,    0,    // WorldValueType universeId
            0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, // WorldValueType galaxyId
            0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11  // WorldValueType planetId
        };

        HelloValueType helloValue{0xDEAD, 0xBEEF};
        WorldValueType worldValue{0, 42, 0x0102030405060708, 0x1112131415161718};

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == true);
        CHECK(testTlvLanguage.world == worldValue);
    }

    SECTION ("Read 2 TLV from buffer: one valid, one ignored") {

        // Construct a test TLV list buffer
        HelloValueType helloValue{0xDEAD, 0xBEEF};

        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            8,    0x00, 0x00, 0x00,                         // HelloValueType size
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            0xEF, 0xBE, 0xAD, 0xBA,                        // TlvTestLanguage::Tags::BadTag
            8,    0x00, 0x00, 0x00,                        // size
            0x34, 0x12, 0x00, 0x00, 0x78, 0x56, 0x00, 0x00 // value
        };

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read 3 TLV from buffer, including an array one") {

        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            8,    0x00, 0x00, 0x00,                         // HelloValueType size
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            24,   0x00, 0x00, 0x00, // TheValueType tag
            20,   0x00, 0x00, 0x00, // TheValueType size
            1,    0,    0,    0,    // TheValueType value 1
            2,    0,    0,    0,    // TheValueType value 2
            3,    0,    0,    0,    // TheValueType value 3
            4,    0,    0,    0,    // TheValueType value 4
            5,    0,    0,    0,    // TheValueType value 5

            230,  182,  1,    0x00, // WorldValueType tag
            21,   0x00, 0x00, 0x00, // WorldValueType size
            0,                      // WorldValueType plankRandomGeneratorSeed
            42,   0,    0,    0,    // WorldValueType universeId
            0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, // WorldValueType galaxyId
            0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11  // WorldValueType planetId
        };

        // Construct a test TLV list buffer
        HelloValueType helloValue{0xDEAD, 0xBEEF};
        std::vector<TheValueType> theValues{{1}, {2}, {3}, {4}, {5}};
        WorldValueType worldValue{0, 42, 0x0102030405060708, 0x1112131415161718};

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

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

    SECTION ("Read 3 TLV from buffer, including multiple 0 sizes") {

        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            8,    0x00, 0x00, 0x00,                         // HelloValueType size
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            24,   0x00, 0x00, 0x00, // TheValueType tag
            0x0,  0x00, 0x00, 0x00, // TheValueType size

            230,  182,  1,    0x00, // WorldValueType tag
            0x00, 0x00, 0x00, 0x00, // WorldValueType ize
        };

        // Construct a test TLV list buffer
        HelloValueType helloValue{0xDEAD, 0xBEEF};

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext() == true);
        CHECK(unpacker.readNext());
        CHECK_THROWS_AS_MSG(
            unpacker.readNext(), TlvUnpack::Exception,
            "Error reading value for "
            "tag 112358: Can not read tlv value: Read failed: end of stream reached");

        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read from buffer including bad size as last element") {
        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            8,    0x00, 0x00, 0x00,                         // HelloValueType size
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            230,  182,  1,    0x00, // WorldValueType tag
            24,   0x00, 0x00, 0x00, // WorldValueType wrong size: size + 3
            0,                      // WorldValueType plankRandomGeneratorSeed
            42,   0,    0,    0,    // WorldValueType universeId
            0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, // WorldValueType galaxyId
            0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11  // WorldValueType planetId
        };

        // Construct a test TLV list buffer
        HelloValueType helloValue{0xDEAD, 0xBEEF};
        WorldValueType worldValue{0, 42, 0x0102030405060708, 0x1112131415161718};

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext() == true);
        CHECK_THROWS_AS_MSG(unpacker.readNext(), TlvUnpack::Exception,
                            "Unable to read tlv: Read failed: end of stream reached");
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read from buffer including bad size as middle element") {

        // Construct a test TLV list buffer
        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            11,   0x00, 0x00, 0x00,                         // HelloValueType wrong size : size + 3
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            230,  182,  1,    0x00, // WorldValueType tag
            21,   0x00, 0x00, 0x00, // WorldValueType size
            0,                      // WorldValueType plankRandomGeneratorSeed
            42,   0,    0,    0,    // WorldValueType universeId
            0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, // WorldValueType galaxyId
            0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11  // WorldValueType planetId
        };

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK_THROWS_AS_MSG(
            unpacker.readNext(), TlvUnpack::Exception,
            "Error reading value for tag 54: The value buffer has not been fully consumed");

        // Obviously because of the invalid length, the next read will fail because of the random
        // tag value:
        CHECK_THROWS(unpacker.readNext());
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == false);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read from buffer with last TLV incomplete: missing value part") {

        // Construct a test TLV list buffer
        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            8,    0x00, 0x00, 0x00,                         // HelloValueType size
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            230,  182,  1,    0x00, // WorldValueType tag
            21,   0x00, 0x00, 0x00, // WorldValueType size
            // missing WorldValueType value
        };

        HelloValueType helloValue{0xDEAD, 0xBEEF};

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext());
        CHECK_THROWS_AS_MSG(unpacker.readNext(), TlvUnpack::Exception,
                            "Unable to read tlv: Read failed: end of stream reached");
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read from buffer with last TLV incomplete: missing value and length") {
        // Construct a test TLV list buffer
        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            8,    0x00, 0x00, 0x00,                         // HelloValueType size
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            230,  182,  1,    0x00, //  WorldValueTypetag
            // missing WorldValueType length and value
        };

        HelloValueType helloValue{0xDEAD, 0xBEEF};

        // Now test the unpacker
        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext() == true);
        CHECK_THROWS_AS_MSG(unpacker.readNext(), TlvUnpack::Exception,
                            "Unable to read tlv: Read failed: end of stream reached");
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION ("Read from buffer with last TLV incomplete: missing tag part and value and length") {
        // Construct a test TLV list buffer
        Buffer buffer{
            54,   0x00, 0x00, 0x00,                         // HelloValueType tag
            8,    0x00, 0x00, 0x00,                         // HelloValueType size
            0xAD, 0xDE, 0x00, 0x00, 0xEF, 0xBE, 0x00, 0x00, // HelloValueType value

            230,  182 // incomplete tag
        };

        HelloValueType helloValue{0xDEAD, 0xBEEF};

        TlvUnpack unpacker(testTlvLanguage, buffer);

        CHECK(unpacker.readNext() == true);
        CHECK_THROWS_AS_MSG(unpacker.readNext(), TlvUnpack::Exception,
                            "Unable to read tlv: Read failed: end of stream reached");
        CHECK(unpacker.readNext() == false);

        CHECK(testTlvLanguage.isHelloValid == true);
        CHECK(testTlvLanguage.hello == helloValue);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }
}