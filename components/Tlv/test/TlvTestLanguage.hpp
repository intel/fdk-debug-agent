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
#pragma once

#include "Tlv/TlvResponseHandlerInterface.hpp"
#include "Tlv/TlvDictionary.hpp"
#include "Tlv/TlvWrapper.hpp"
#include "Tlv/TlvVectorWrapper.hpp"

using namespace debug_agent::tlv;

struct HelloValueType {
    int whoTalk;
    int whoListen;

    bool operator==(const HelloValueType &other) const
    {
        return whoTalk == other.whoTalk
            && whoListen == other.whoListen;
    }
};

struct TheValueType {
    int itMustBe42;

    bool operator==(const TheValueType &other) const
    {
        return itMustBe42 == other.itMustBe42;
    }
};

struct WorldValueType {
    char plankRandomGeneratorSeed;
    int universeId;
    long long galaxyId;
    long long planetId;

    bool operator==(const WorldValueType &other) const
    {
        return plankRandomGeneratorSeed == other.plankRandomGeneratorSeed
            && universeId == other.universeId
            && galaxyId == other.galaxyId
            && planetId == planetId;
    }
};

class TlvTestLanguage final: public TlvResponseHandlerInterface
{
public:
    HelloValueType hello;
    bool isHelloValid;

    std::vector<TheValueType> the;

    WorldValueType world;
    bool isWorldValid;

    enum class Tags: unsigned int {
        Hello = 54,
        The = 24,
        World = 112358
    };
    static const unsigned int aTagIdWhichIsNotInTheLanguageTagsList = 42;

    TlvTestLanguage():
        mTestLanguageTlvMap(),
        mLanguageDictionary(mTestLanguageTlvMap)
    {
        helloWrapperPointer = new TlvWrapper<HelloValueType>(hello, isHelloValid);
        mTestLanguageTlvMap[Tags::Hello] =
            std::unique_ptr<TlvWrapperInterface>(helloWrapperPointer);

        theWrapperPointer = new TlvVectorWrapper<TheValueType>(the);
        mTestLanguageTlvMap[Tags::The] =
            std::unique_ptr<TlvWrapperInterface>(theWrapperPointer);

        worldWrapperPointer = new TlvWrapper<WorldValueType>(world, isWorldValid);
        mTestLanguageTlvMap[Tags::World] =
            std::unique_ptr<TlvWrapperInterface>(worldWrapperPointer);
    }

    const TlvDictionaryInterface &getTlvDictionary() const NOEXCEPT override
    {
        return mLanguageDictionary;
    }

    TlvWrapperInterface *helloWrapperPointer;
    TlvWrapperInterface *theWrapperPointer;
    TlvWrapperInterface *worldWrapperPointer;

private:
    TlvDictionary<Tags>::TlvMap mTestLanguageTlvMap;
    TlvDictionary<Tags> mLanguageDictionary;
};