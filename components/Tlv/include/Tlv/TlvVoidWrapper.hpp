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
#include <limits>

namespace debug_agent
{
namespace tlv
{
/**
 * Real implementation of a TlvWrapperInterface to explicitly ignore a tag in a dictionary, avoiding
 * * an exception for unknown tag in the TLV list if this tag is found. A TlvVoidWrapper does not
 * update any shadow variable and accepts any TLV length (including 0).
 * @see TlvWrapperInterface
 */
class TlvVoidWrapper final : public TlvWrapperInterface
{
public:
    TlvVoidWrapper() {}

    void readFrom(const util::Buffer &binarySource) override {}

    virtual void invalidate() noexcept override {}
};
}
}