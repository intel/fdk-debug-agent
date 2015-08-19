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

#include "Tlv/TlvWrapperInterface.hpp"
#include <algorithm>

namespace debug_agent
{
namespace tlv
{
/**
 * Real implementation of a TlvWrapperInterface for a simple value.
 * The validity of a shadow variable managed by a TlvWrapper is implemented by a bool flag.
 * @tparam ValueType the type of the value managed by this TlvWrapper
 * @see TlvWrapperInterface
 */
template <typename ValueType>
class TlvWrapper final: public TlvWrapperInterface
{
public:
    /**
     * @param[in] value the reference to the shadow runtime variable
     * @param[in] valid the reference to the shadow runtime variable valid flag
     */
    TlvWrapper(ValueType &value, bool &valid):
        mValue(value),
        mValid(valid)
    {
        invalidate();
    }

    virtual bool isValidSize(size_t binaryValueSize) const NOEXCEPT override
    {
        return binaryValueSize == sizeof(ValueType);
    }

    virtual void readFrom(const char *binarySource, size_t binaryValueSize) override
    {
        if (!isValidSize(binaryValueSize)) {

            throw Exception("Invalid binary size for TLV value read");
        }
        mValue = *(reinterpret_cast<const ValueType *>(binarySource));
        mValid = true;
    }

    virtual void invalidate() NOEXCEPT override
    {
        mValid = false;
    }

private:
    ValueType &mValue;
    bool &mValid;
};

}
}