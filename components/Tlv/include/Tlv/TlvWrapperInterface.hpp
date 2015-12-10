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
#include <cstddef>
#include <stdexcept>

namespace debug_agent
{
namespace tlv
{
/**
 * Defines the requirement for a TlvWrapper.
 * A TlvWrapper is specialized for a given TLV Value Type using a template class parameter.
 * The TlvWrapperInterface allows to abstract this specialization in order to have polymorphic
 * collection of TlvWrapper.
 * A TlvWrapper is the link between the TLV in a binary form from a binary TLV list buffer and
 * its shadow representation in a runtime variable.
 * When a binary TLV list is read by the TlvUnpack, it uses the associated TlvWrapper in order
 * to reflect the binary value from the buffer to the shadow runtime variable.
 * The wrapper also provides a way to invalidate the shadow variable.
 * @see TlvWrapper
 * @see TlvUnpack
 */
class TlvWrapperInterface
{
public:
    class Exception final : public std::logic_error
    {
    public:
        explicit Exception(const std::string &what) : std::logic_error(what) {}
    };

    /**
     * @param[in] binaryValueSize a binary size to be validated
     * @return true if the binary value size is valid for the shadow variable
     */
    virtual bool isValidSize(size_t binaryValueSize) const NOEXCEPT = 0;

    /**
     * Read the value from the binary source provided and reflect it to the shadow runtime variable.
     * The caller must ensure there are enough chars from binarySource to read the complete value.
     * @param[in] binarySource the memory from which the value will be read
     * @param[in] binaryValueSize the binary size of the value to be read
     * @throw TlvWrapperInterface::Exception
     * @see isValidSize()
     */
    virtual void readFrom(const char *binarySource, size_t binaryValueSize) = 0;

    /**
     * Invalidate the shadow variable.
     */
    virtual void invalidate() NOEXCEPT = 0;

    virtual ~TlvWrapperInterface() {}
};
}
}