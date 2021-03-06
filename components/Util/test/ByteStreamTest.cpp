/*
 * Copyright (c) 2015-2016, Intel Corporation
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
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include <array>
#include <type_traits>
#include <catch.hpp>

using namespace debug_agent::util;

/* This structure implements the toStream() and fromStream() method */
struct TestStruct
{
    uint32_t a;
    uint32_t b;

    TestStruct() = default;
    TestStruct(uint32_t _a, uint32_t _b) : a(_a), b(_b) {}

    void fromStream(ByteStreamReader &reader)
    {
        reader.read(a);
        reader.read(b);
    }

    void toStream(ByteStreamWriter &writer) const
    {
        writer.write(a);
        writer.write(b);
    }

    bool operator==(const TestStruct &other) const { return a == other.a && b == other.b; }
};

const Buffer expectedBuffer = {
    1,          // 1 as uint8
    2, 0, 0, 0, // 2 as uint32
    2, 0, 0, 0, // array size
    1, 0, 0, 0, // #0 array element
    2, 0, 0, 0, // #0 array element
    2, 0, 0, 0, // array size
    1, 0, 0, 0, // #0 array element field a
    2, 0, 0, 0, // #0 array element field b
    3, 0, 0, 0, // #1 array element field a
    4, 0, 0, 0, // #1 array element field b
};

TEST_CASE("Byte stream writer")
{
    MemoryByteStreamWriter writer;

    writer.write(static_cast<uint8_t>(1));
    writer.write(static_cast<uint32_t>(2));

    std::vector<uint32_t> v;
    v.push_back(1);
    v.push_back(2);

    writer.writeVector<uint32_t>(v);

    std::vector<TestStruct> structs;
    structs.push_back(TestStruct(1, 2));
    structs.push_back(TestStruct(3, 4));

    writer.writeVector<uint32_t>(structs);

    CHECK(writer.getBuffer() == expectedBuffer);
}

TEST_CASE("Byte stream reader")
{
    MemoryByteStreamReader reader(expectedBuffer);

    uint8_t v1;
    reader.read(v1);
    CHECK(v1 == 1);

    uint32_t v2;
    reader.read(v2);
    CHECK(v2 == 2);

    std::vector<uint32_t> v;
    reader.readVector<uint32_t>(v);
    CHECK(v == std::vector<uint32_t>({1, 2}));

    std::vector<TestStruct> structs;
    reader.readVector<uint32_t>(structs);
    CHECK(structs == std::vector<TestStruct>({{1, 2}, {3, 4}}));
}

TEST_CASE("Byte stream reader : end of stream")
{
    Buffer v{1}; /* Adding only one byte */

    MemoryByteStreamReader reader(v);

    /* Checking end of stream exception */
    uint16_t myInt;
    CHECK_THROWS_AS(reader.read(myInt), MemoryByteStreamReader::Exception);

    /* Checking that subsequent calls to isEOS() returns true */
    CHECK(reader.isEOS());
}

TEST_CASE("Byte stream reader : enums")
{
    enum class Enum8 : uint8_t
    {
        val1,
        val2
    };
    static_assert(sizeof(Enum8) == 1, "Wrong Enum8 size");

    enum class Enum16 : uint16_t
    {
        val1,
        val2
    };
    static_assert(sizeof(Enum16) == 2, "Wrong Enum16 size");

    enum class Enum32 : uint32_t
    {
        val1,
        val2,
        val3
    };
    static_assert(sizeof(Enum32) == 4, "Wrong Enum32 size");

    MemoryByteStreamWriter writer;
    writer.write(Enum8::val1);
    writer.write(Enum16::val2);
    writer.write(Enum32::val3);

    /* Checking that all enum values are encoding using the type "EnumEncodingType" */
    REQUIRE(writer.getBuffer().size() == 3 * sizeof(EnumEncodingType));

    /* Reading */
    Enum8 enum8Value;
    Enum16 enum16Value;
    Enum32 enum32Value;

    MemoryByteStreamReader reader(writer.getBuffer());
    reader.read(enum8Value);
    reader.read(enum16Value);
    reader.read(enum32Value);

    CHECK(enum8Value == Enum8::val1);
    CHECK(enum16Value == Enum16::val2);
    CHECK(enum32Value == Enum32::val3);
}

TEST_CASE("Byte Stream reader/writer: arrays")
{
    uint8_t simpleArray[5] = {1, 2, 3, 4, 5};
    struct
    {
        uint32_t i;
        int16_t j;
        void toStream(ByteStreamWriter &writer) const
        {
            writer.write(i);
            writer.write(j);
        }
        void fromStream(ByteStreamReader &reader)
        {
            reader.read(i);
            reader.read(j);
        }
    } structArray[3] = {{0, 1}, {32, 16}, {0xabcdef01, 0x7fff}};
    std::array<uint16_t, 2> stdArray = {0x1337, 0x4242};

    // clang-format off
    const Buffer expected = {
        1, 2, 3, 4, 5, // simpleValue
        0x00, 0x00, 0x00, 0x00, // 0 as uint32
        0x01, 0x00, // 1 as int16
        0x20, 0x00, 0x00, 0x00, // 32 as uint32
        0x10, 0x00, // 16 as int16
        0x01, 0xef, 0xcd, 0xad,
        0xff, 0x7f,
        0x37, 0x13, // std array's first slot
        0x42, 0x42 // second slot
    };
    // clang-format on

    MemoryByteStreamWriter writer;
    writer.write(simpleArray);
    writer.write(structArray);
    writer.write(stdArray);

    MemoryByteStreamReader reader(writer.getBuffer());

    for (const auto expected : simpleArray) {
        Buffer::value_type back;
        reader.read(back);
        CHECK(back == expected);
    }
    for (const auto expected : structArray) {
        std::remove_const<decltype(expected)>::type back;
        reader.read(back);
        CHECK(back.i == expected.i);
        CHECK(back.j == expected.j);
    }

    decltype(stdArray) back;
    reader.read(back);
    CHECK(back == stdArray);
}
