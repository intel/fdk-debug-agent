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
#include "Tlv/TlvWrapper.hpp"
#include "Util/Buffer.hpp"
#include "catch.hpp"

using namespace debug_agent::tlv;
using namespace debug_agent::util;

struct ATestValueType
{
    int anIntField;
    char aCharField;
    short aShortField;

    bool operator==(const ATestValueType &other)
    {
        return anIntField == other.anIntField && aCharField == other.aCharField &&
               aShortField == other.aShortField;
    }

    void fromStream(ByteStreamReader &reader)
    {
        reader.read(anIntField);
        reader.read(aCharField);
        reader.read(aShortField);
    }
};

TEST_CASE("TlvWrapper", "[WrapperRead]")
{
    ATestValueType testValue;
    bool testValueIsValid = false;
    TlvWrapper<ATestValueType> tlvWrapper(testValue, testValueIsValid);

    CHECK(testValueIsValid == false);

    Buffer buffer{210, 4, 0, 0, 56, 21, 3};

    ATestValueType valueToBeRead = {1234, 56, 789};

    tlvWrapper.readFrom(buffer);
    CHECK(testValue == valueToBeRead);
    CHECK(testValueIsValid == true);

    tlvWrapper.invalidate();
    CHECK(testValueIsValid == false);
}
