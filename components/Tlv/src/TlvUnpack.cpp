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

TlvUnpack::TlvUnpack(const TlvResponseHandlerInterface &responseHandler, const util::Buffer &buffer)
    : mResponseHandler(responseHandler), mBufferReader(buffer)
{
    mResponseHandler.getTlvDictionary().invalidateAll();
}

bool TlvUnpack::readNext()
{
    if (mBufferReader.isEOS()) {
        return false;
    }

    util::Buffer valueBuffer;
    uint32_t tagFromBuffer;

    try {
        uint32_t lengthFromBuffer;
        mBufferReader.read(tagFromBuffer);
        mBufferReader.read(lengthFromBuffer);

        /* It is dangerous to resize the buffer to "lengthFromBuffer", because this size is
         * provided by an external component, and if this size is huge this will lead to memory
         * allocation failure.
         *
         * To avoid this allocation, reading one byte at time, in this way end of stream
         * will be reached before memory saturation.
         */
        for (std::size_t i = 0; i < lengthFromBuffer; i++) {
            uint8_t byte;
            mBufferReader.read(byte);
            valueBuffer.push_back(byte);
        }
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Unable to read tlv: " + std::string(e.what()));
    }

    // Is Tag in the dictionary ?
    TlvWrapperInterface *tlvWrapper =
        mResponseHandler.getTlvDictionary().getTlvWrapperForTag(tagFromBuffer);
    if (tlvWrapper == nullptr) {

        // Tag is unknown
        throw Exception("Cannot parse unknown tag " + std::to_string(tagFromBuffer));
    }

    // Read value
    try {
        tlvWrapper->readFrom(valueBuffer);
    } catch (TlvWrapperInterface::Exception &e) {
        throw Exception("Error reading value for tag " + std::to_string(tagFromBuffer) + ": " +
                        std::string(e.what()));
    }

    return true;
}
}
}