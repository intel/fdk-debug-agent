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

#include "Util/RingBuffer.hpp"
#include "Util/AssertAlways.hpp"
#include <catch.hpp>
#include <future>
#include <array>
#include <algorithm>
#include <stdexcept>

template <typename T>
using alias = T;

using namespace debug_agent::util;

// This macro writes parameters (Byte *src, std::size_ count) for method write() or
// writeNonBlocking() of a RingBuffer class
#define ARR(count, ...) std::array<uint8_t, count>{__VA_ARGS__}.data(), count

static bool startsWith(const Buffer &buffer, const Buffer &prefix)
{
    ASSERT_ALWAYS(buffer.size() >= prefix.size());
    return std::equal(prefix.begin(), prefix.end(), buffer.begin());
}

TEST_CASE("RingBuffer: non blocking")
{
    RingBuffer buffer(5);
    buffer.open();

    INFO("Writing 3 bytes: all are written");
    REQUIRE(buffer.writeNonBlocking(ARR(3, 0, 1, 2)) == 3);

    INFO("Writing 3 bytes more: only 2 are written, the buffer is full");
    REQUIRE(buffer.writeNonBlocking(ARR(3, 3, 4, 5)) == 2);

    INFO("Cannot write more data");
    REQUIRE(buffer.writeNonBlocking(ARR(1, 6)) == 0);

    Buffer out(10);

    INFO("Reading 3 bytes : ok");
    REQUIRE(buffer.readNonBlocking(out.data(), 3) == 3);
    REQUIRE(startsWith(out, Buffer{0, 1, 2}));

    INFO("Reading 3 bytes more: only 2 are read (buffer is empty)");
    REQUIRE(buffer.readNonBlocking(out.data(), 3) == 2);
    REQUIRE(startsWith(out, Buffer{3, 4}));

    INFO("Reading again: no byte read");
    REQUIRE(buffer.readNonBlocking(out.data(), 2) == 0);

    INFO("Making wrap the producer producer position");
    REQUIRE(buffer.writeNonBlocking(ARR(3, 5, 6, 7)) == 3);

    INFO("Making wrap the consumer position");
    REQUIRE(buffer.readNonBlocking(out.data(), 3) == 3);
    REQUIRE(startsWith(out, Buffer{5, 6, 7}));

    INFO("Checking that the buffer is empty");
    REQUIRE(buffer.getUsedSize() == 0);
}

TEST_CASE("RingBuffer: blocking and multithreading")
{
    RingBuffer buffer(5);
    buffer.open();

    // Writing a lot of data into a small ring buffer in a dedicated thread
    auto future = std::async(std::launch::async, [&buffer] {
        for (uint8_t i = 0; i < 100; i++) {
            Buffer in(i, i);
            // Do not use catch macro outside the main test thread
            if (!buffer.writeBlocking(in.data(), in.size())) {
                throw std::runtime_error("writeBlocking: expected true as returned value");
            }
        }
        buffer.close();
    });

    INFO("Reading written data in another thread and checking its validity");
    for (uint8_t i = 0; i < 100; i++) {
        Buffer out(i);
        CHECK(buffer.readBlocking(out.data(), out.size()));
        REQUIRE(out == Buffer(i, i));
    }

    INFO("Checking that the buffer is empty");
    REQUIRE(buffer.getUsedSize() == 0);

    INFO("Checking that producer thread has not thrown an exception");
    future.get();
}

TEST_CASE("RingBuffer: close stops blocking methods")
{
    RingBuffer buffer(5);
    buffer.open();

    // Note: the closing may happen before or during xxxxBlocking() call
    auto future = std::async(std::launch::async, [&buffer] { buffer.close(); });

    WHEN ("Reading a RB while closing it") {
        Buffer out(10);
        CHECK_FALSE(buffer.readBlocking(out.data(), 5));
        future.get();
    }
    WHEN ("Writing a RB while closing it") {
        Buffer in(10, 0);
        CHECK_FALSE(buffer.writeBlocking(in.data(), in.size()));
        future.get();
    }
}