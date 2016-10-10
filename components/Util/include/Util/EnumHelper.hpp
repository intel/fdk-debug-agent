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
            return "Unknown value '" + std::to_string(static_cast<int32_t>(value)) + "' of " +
                   getEnumTypeName();
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

    const std::map<EnumType, std::string> &getEnumToStringMap() const { return mEnumToStringMap; }

    const std::map<std::string, EnumType> &getStringToEnumMap() const { return mStringToEnumMap; }

private:
    EnumHelper(const EnumHelper &) = delete;
    EnumHelper &operator=(const EnumHelper &) = delete;

    std::map<EnumType, std::string> mEnumToStringMap;
    std::map<std::string, EnumType> mStringToEnumMap;

    static std::string getEnumTypeName() { return typeid(EnumType).name(); }
};
}
}
