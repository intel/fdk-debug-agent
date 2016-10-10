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

#include "Tlv/TlvDictionaryInterface.hpp"
#include <map>
#include <memory>

namespace debug_agent
{
namespace tlv
{
/**
 * Real implementation of a TlvDictionaryInterface.
 * @tparam TagsEnumClass the type of the enum class defining the TLV tag list
 * @see TlvDictionaryInterface
 */
template <typename TagsEnumClass>
class TlvDictionary final : public TlvDictionaryInterface
{
public:
    using TlvMap = std::map<TagsEnumClass, std::unique_ptr<TlvWrapperInterface>>;

    /**
     * @param[in] tlvMap the map which associates a TlvWrapepr to a TLV tag
     */
    TlvDictionary(TlvMap tlvMap) : mTlvMap(std::move(tlvMap)) {}

    TlvDictionary<TagsEnumClass> &operator=(TlvDictionary<TagsEnumClass> &&) = default;
    TlvDictionary<TagsEnumClass>(TlvDictionary<TagsEnumClass> &&) = default;

    TlvWrapperInterface *getTlvWrapperForTag(unsigned int tag) const noexcept override
    {
        return getTlvWrapperForTag(static_cast<TagsEnumClass>(tag));
    }

    TlvWrapperInterface *getTlvWrapperForTag(TagsEnumClass tag) const noexcept
    {
        typename TlvMap::const_iterator it = mTlvMap.find(tag);

        if (it != mTlvMap.end()) {

            return it->second.get();
        } else {

            return nullptr;
        }
    }

    virtual void invalidateAll() const noexcept override
    {
        for (auto &dictionaryEntry : mTlvMap) {

            if (dictionaryEntry.second != nullptr) {

                dictionaryEntry.second->invalidate();
            }
        }
    }

private:
    TlvMap mTlvMap;
};
}
}
