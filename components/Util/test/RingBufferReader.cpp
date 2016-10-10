/*
 * Copyright (c) 2016, Intel Corporation
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
