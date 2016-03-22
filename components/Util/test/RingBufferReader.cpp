/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include <Util/RingBufferReader.hpp>

#include <catch.hpp>

#include <deque>

using namespace debug_agent::util;

SCENARIO("Normal operation", "[ring-buffer]")
{
    Buffer buff = {'a', 'b', 'c', 'd', 'e'};
    std::deque<RingBufferReader::LinearPosition> positions;
    auto getProdPos = [&positions] {
        auto pos = positions.at(0);
        positions.pop_front();
        return pos;
    };
    Buffer out;
    RingBufferReader ringBuff(buff.data(), buff.size(), getProdPos);

    GIVEN ("A starving ring buffer") {
        positions = {0};
        THEN ("Read should succeed") {
            CHECK_NOTHROW(ringBuff.readAvailable(out));
            AND_THEN ("Out buffer should be left unmodified") {
                CHECK(out.empty());
            }
        }
    }
    WHEN ("3 bytes are available") {
        positions = {3};
        THEN ("Producer positions should have been updated") {
            CHECK_NOTHROW(ringBuff.readAvailable(out));
            CHECK(out == (Buffer{'a', 'b', 'c'}));
            CHECK(positions.size() == 0);
            WHEN ("3 bytes more are available") {
                positions = {6};
                CHECK_NOTHROW(ringBuff.readAvailable(out));
                CHECK(out == (Buffer{'a', 'b', 'c', 'd', 'e', 'a'}));
                CHECK(positions.size() == 0);
            }
        }
    }

    GIVEN ("Overflow condition detection") {
        positions = {buff.size() + 1};
        THEN ("Read should fail by returning throwing") {
            CHECK_THROWS_AS(ringBuff.readAvailable(out), RingBufferReader::Exception);
            AND_THEN ("Out buffer should be left unmodified") {
                CHECK(out.empty());
            }
        }
    }

    GIVEN ("A pre-filled ring buffer") {
        struct Test
        {
            std::string name;
            size_t position;
            std::string result;
        };
        const Test tests[] = {
            {"Read first byte", 1, "a"},
            {"Read the rest", buff.size(), "bcde"},
            {"Buffer loop", buff.size() + 2, "ab"},
            {"Read over buffer end", buff.size() * 2 + 2, "cdeab"},
        };

        for (auto test : tests) {
            INFO("Read name: " + test.name);
            INFO("Original buffer content: " + Catch::toString(out));

            Buffer expectedBuff = out;
            expectedBuff.insert(end(expectedBuff), begin(test.result), end(test.result));
            positions = {test.position};

            CHECK_NOTHROW(ringBuff.readAvailable(out));
            INFO("Expected insertion of \"" + test.result + '"');
            CHECK(out == expectedBuff);
            CHECK(positions.empty());
        }
    }
    // Test that the read buffer has retrieved producer position as many time as expected
    CHECK(positions.size() == 0);
}
