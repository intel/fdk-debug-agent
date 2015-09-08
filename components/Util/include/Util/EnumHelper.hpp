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

#include <string>
#include <map>
#include <cassert>
#include <typeinfo>

namespace debug_agent
{
namespace util
{

/** This class provides services to an enum supplied as template parameter:
 * - conversion from & to string
 * - value validation
 */
template <typename EnumType>
class EnumHelper final
{
public:
    /** @param enumNames a map that associates names to enum values */
    EnumHelper(const std::map<EnumType, std::string> &enumNames)
    {
        mEnumToStringMap = enumNames;
        for (auto entry : enumNames) {

            /* Filling the string -> enum map */
            assert(mStringToEnumMap.find(entry.second) == mStringToEnumMap.end());
            mStringToEnumMap[entry.second] = entry.first;
        }
    }

    /** Convert an enum value to string. If the value is invalid, the message "Unknown value 'X' of
     * enum <enum_name>" is returned. */
    std::string toString(EnumType value) const
    {
        auto it = mEnumToStringMap.find(value);
        if (it == mEnumToStringMap.end()) {
            return "Unknown value '" + std::to_string(static_cast<int32_t>(value)) +
                "' of " + getEnumTypeName();
        }
        return it->second;
    }

    /** Convert a string to an enum value. If the value is not found, the method return false. */
    bool fromString(const std::string &name, EnumType &value) const
    {
        auto it = mStringToEnumMap.find(name);
        if (it == mStringToEnumMap.end()) {
            return false;
        }
        value = it->second;
        return true;
    }

    /** Check if an enum value is valid */
    bool isValid(EnumType value) const
    {
        auto it = mEnumToStringMap.find(value);
        return it != mEnumToStringMap.end();
    }

private:
    EnumHelper(const EnumHelper&) = delete;
    EnumHelper& operator=(const EnumHelper&) = delete;

    std::map<EnumType, std::string> mEnumToStringMap;
    std::map<std::string, EnumType> mStringToEnumMap;

    static std::string getEnumTypeName()
    {
        return typeid(EnumType).name();
    }
};

}
}
