/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
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
    ByteStreamWriter writer;

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
    ByteStreamReader reader(expectedBuffer);

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

    ByteStreamReader reader(v);

    /* Checking end of stream exception */
    uint16_t myInt;
    CHECK_THROWS_AS(reader.read(myInt), ByteStreamReader::Exception);

    /* Checking that subsequent calls to isEOS() returns true */
    CHECK(reader.isEOS());
}