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
#include "Tlv/TlvUnpack.hpp"
#include <algorithm>

namespace debug_agent
{
namespace tlv
{

TlvUnpack::TlvUnpack(const TlvResponseHandlerInterface &responseHandler,
                     const char *tlvBuffer,
                     std::size_t tlvBufferSize):
    mResponseHandler(responseHandler),
    mTlvBuffer(tlvBuffer),
    mTlvBufferSize(tlvBufferSize),
    mReadIndex(0)
{
    if (mTlvBuffer == nullptr) {

        throw Exception("Null pointer");
    }
    mResponseHandler.getTlvDictionary().invalidateAll();
}

bool TlvUnpack::readNext()
{
    uint32_t tagFromBuffer;
    uint32_t lengthFromBuffer;

    if (mReadIndex >= mTlvBufferSize) {

        return false;
    }

    if (!popUint32(tagFromBuffer) || !popUint32(lengthFromBuffer)) {

        // Nothing more can be read from buffer
        mReadIndex = mTlvBufferSize;
        throw Exception("Incomplete TLV at end of buffer");
    }

    unsigned int tag = static_cast<unsigned int>(tagFromBuffer);
    std::size_t length = static_cast<std::size_t>(lengthFromBuffer);

    // Is Tag in the dictionary ?
    TlvWrapperInterface *tlvWrapper = mResponseHandler.getTlvDictionary().getTlvWrapperForTag(tag);

    if (tlvWrapper == nullptr) {

        // Tag is unknown: ignore this value from buffer
        mReadIndex += length;
        throw Exception("Cannot parse unknown tag " + std::to_string(tag));
    }
    // Read value
    if (mTlvBufferSize - mReadIndex >= length) {

        try
        {
            tlvWrapper->readFrom(mTlvBuffer + mReadIndex, length);
        }
        catch (TlvWrapperInterface::Exception &e)
        {
            throw Exception("Error reading value for tag "
                + std::to_string(tag)
                + " (size " + std::to_string(length) + " bytes)"
                + ": " + e.what());
        }
        mReadIndex += length;
    } else {

        mReadIndex += length;
        throw Exception("Incomplete value for tag " + std::to_string(tag));
    }

    return true;
}

bool TlvUnpack::popUint32(uint32_t &value)
{
    if (mTlvBufferSize - mReadIndex >= sizeof(uint32_t)) {

        value = *(reinterpret_cast<const uint32_t *>(mTlvBuffer + mReadIndex));

        mReadIndex += sizeof(uint32_t);
        return true;
    } else {

        return false;
    }
}

}
}