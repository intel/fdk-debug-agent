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
#include "Tlv/TlvVectorWrapper.hpp"
#include "catch.hpp"

using namespace debug_agent::tlv;

struct ATestValueType
{
    int anIntField;
    char aCharField;
    short aShortField;

    bool operator==(const ATestValueType &other) const
    {
        return anIntField == other.anIntField
            && aCharField == other.aCharField
            && aShortField == other.aShortField;
    }
};

TEST_CASE("TlvVectorWrapper", "[VectorWrapperRead]")
{
    std::vector<ATestValueType> testValue;

    TlvVectorWrapper<ATestValueType> tlvVectorWrapper(testValue);

    CHECK(testValue.size() == 0);
    CHECK(tlvVectorWrapper.isValidSize(sizeof(ATestValueType)) == true);
    CHECK(tlvVectorWrapper.isValidSize(sizeof(ATestValueType) + 1) == false);
    CHECK(tlvVectorWrapper.isValidSize(sizeof(ATestValueType) - 1) == false);

    CHECK(tlvVectorWrapper.isValidSize(sizeof(ATestValueType) * 2) == true);
    CHECK(tlvVectorWrapper.isValidSize(sizeof(ATestValueType) * 2 + 1) == false);
    CHECK(tlvVectorWrapper.isValidSize(sizeof(ATestValueType) * 2 - 1) == false);

    std::vector<ATestValueType> valueToBeRead;
    valueToBeRead = { { 1234, 56, 789},
                      { 987, 65, 4321},
                      { 5484, 47, 754} };
    const char *rawValue = reinterpret_cast<const char *>(valueToBeRead.data());

    tlvVectorWrapper.readFrom(rawValue, sizeof(ATestValueType) * valueToBeRead.size());

    CHECK(testValue == valueToBeRead);

    tlvVectorWrapper.invalidate();
    CHECK(testValue.size() == 0);
}