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
#include "Util/ByteStreamReader.hpp"
#include <algorithm>
#include <vector>

namespace debug_agent
{
namespace tlv
{
/**
 * Real implementation of a TlvWrapperInterface for a Vector of value as TLV Value.
 * The validity of a shadow variable managed by a TlvVectorWrapper is intrinsic: unless the vector
 * size is null, the vector is valid.
 * @tparam ValueType the type of the values managed by this TlvVectorWrapper
 * @see TlvWrapperInterface
 */
template <typename ValueType>
class TlvVectorWrapper final : public TlvWrapperInterface
{
public:
    /**
     * @param[in] values the reference to the shadow runtime variable
     */
    TlvVectorWrapper(std::vector<ValueType> &values) : mValues(values) { invalidate(); }

    void readFrom(const util::Buffer &binarySource) override
    {
        mValues.clear();
        try {
            util::ByteStreamReader reader(binarySource);
            while (!reader.isEOS()) {
                ValueType value;
                reader.read(value);
                mValues.push_back(value);
            }
        } catch (util::ByteStreamReader::Exception &e) {
            throw Exception("Can not read array element: " + std::string(e.what()));
        }
    }

    virtual void invalidate() noexcept override { mValues.clear(); }

private:
    std::vector<ValueType> &mValues;
};
}
}
