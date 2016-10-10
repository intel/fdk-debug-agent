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
#pragma once

#include "Tlv/TlvResponseHandlerInterface.hpp"
#include "Tlv/TlvDictionary.hpp"
#include "Tlv/TlvWrapper.hpp"
#include "Tlv/TlvVoidWrapper.hpp"
#include "Tlv/TlvVectorWrapper.hpp"

using namespace debug_agent::tlv;
using namespace debug_agent::util;

struct HelloValueType
{
    int whoTalk;
    int whoListen;

    bool operator==(const HelloValueType &other) const
    {
        return whoTalk == other.whoTalk && whoListen == other.whoListen;
    }

    void fromStream(ByteStreamReader &reader)
    {
        reader.read(whoTalk);
        reader.read(whoListen);
    }
};

struct TheValueType
{
    int itMustBe42;

    bool operator==(const TheValueType &other) const { return itMustBe42 == other.itMustBe42; }

    void fromStream(ByteStreamReader &reader) { reader.read(itMustBe42); }
};

struct WorldValueType
{
    char plankRandomGeneratorSeed;
    int universeId;
    long long galaxyId;
    long long planetId;

    bool operator==(const WorldValueType &other) const
    {
        return plankRandomGeneratorSeed == other.plankRandomGeneratorSeed &&
               universeId == other.universeId && galaxyId == other.galaxyId &&
               planetId == other.planetId;
    }

    void fromStream(ByteStreamReader &reader)
    {
        reader.read(plankRandomGeneratorSeed);
        reader.read(universeId);
        reader.read(galaxyId);
        reader.read(planetId);
    }
};

class TlvTestLanguage final : public TlvResponseHandlerInterface
{
public:
    HelloValueType hello;
    bool isHelloValid;

    std::vector<TheValueType> the;

    WorldValueType world;
    bool isWorldValid;

    enum class Tags : unsigned int
    {
        Hello = 54,
        The = 24,
        World = 112358,
        BadTag = 0xBAADBEEF
    };
    static const unsigned int aTagIdWhichIsNotInTheLanguageTagsList = 42;

    TlvTestLanguage() : mLanguageDictionary(mkMap()) {}

    const TlvDictionaryInterface &getTlvDictionary() const noexcept override
    {
        return mLanguageDictionary;
    }

    TlvWrapperInterface *getReferenceWrapper(Tags tag)
    {
        return mLanguageDictionary.getTlvWrapperForTag(tag);
    }

private:
    TlvDictionary<Tags>::TlvMap mkMap()
    {
        TlvDictionary<Tags>::TlvMap mTestLanguageTlvMap;
        mTestLanguageTlvMap[Tags::Hello] =
            std::make_unique<TlvWrapper<HelloValueType>>(hello, isHelloValid);

        mTestLanguageTlvMap[Tags::The] = std::make_unique<TlvVectorWrapper<TheValueType>>(the);

        mTestLanguageTlvMap[Tags::World] =
            std::make_unique<TlvWrapper<WorldValueType>>(world, isWorldValid);

        mTestLanguageTlvMap[Tags::BadTag] = std::make_unique<TlvVoidWrapper>();

        return mTestLanguageTlvMap;
    }
    TlvDictionary<Tags> mLanguageDictionary;
};
