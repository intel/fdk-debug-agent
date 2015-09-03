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
#include "catch.hpp"

TEST_CASE("TlvDictionary", "[Dictionary]")
{
    TlvTestLanguage testTlvLanguage;

    const TlvDictionaryInterface &dictionary = testTlvLanguage.getTlvDictionary();

    SECTION("Check all value are invalidated by default") {

        CHECK(testTlvLanguage.isHelloValid == false);
        CHECK(testTlvLanguage.the.size() == 0);
        CHECK(testTlvLanguage.isWorldValid == false);
    }

    SECTION("Check Dictionary TlvWrapper retrieving") {

        TlvWrapperInterface *tlvWrapper;

        // Retrieve the HELLO TLV wrapper
        tlvWrapper = dictionary.getTlvWrapperForTag
            (static_cast<unsigned int>(TlvTestLanguage::Tags::Hello));
        CHECK(tlvWrapper != nullptr);
        CHECK(tlvWrapper == testTlvLanguage.getReferenceWrapper(TlvTestLanguage::Tags::Hello));

        // Retrieve the THE TLV wrapper
        tlvWrapper = dictionary.getTlvWrapperForTag
            (static_cast<unsigned int>(TlvTestLanguage::Tags::The));
        CHECK(tlvWrapper != nullptr);
        CHECK(tlvWrapper == testTlvLanguage.getReferenceWrapper(TlvTestLanguage::Tags::The));

        // Retrieve the WORLD TLV wrapper
        tlvWrapper = dictionary.getTlvWrapperForTag
            (static_cast<unsigned int>(TlvTestLanguage::Tags::World));
        CHECK(tlvWrapper != nullptr);
        CHECK(tlvWrapper == testTlvLanguage.getReferenceWrapper(TlvTestLanguage::Tags::World));

        // Check that unknown tag does not retrieve any TLV wrapper
        tlvWrapper = dictionary.getTlvWrapperForTag
            (static_cast<unsigned int>(TlvTestLanguage::aTagIdWhichIsNotInTheLanguageTagsList));
        CHECK(tlvWrapper == nullptr);
    }
}