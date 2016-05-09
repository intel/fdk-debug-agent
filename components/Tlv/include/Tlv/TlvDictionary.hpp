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
