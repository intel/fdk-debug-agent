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

#include "Tlv/TlvResponseHandlerInterface.hpp"
#include "Util/ByteStreamReader.hpp"
#include <inttypes.h>
#include <string>
#include <stdexcept>

namespace debug_agent
{
namespace tlv
{
/**
 * Allow to read a binary TLV list and update the corresponding value using the appropriate
 * TlvResponseHandlerInterface.
 */
class TlvUnpack final
{
public:
    struct Exception final : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /**
     * Create a TlvUnpack used to read TLV from a TLV list in a binary buffer. The TLV dictionary
     * of the TlvResponseHandlerInterface is reset: each of its TLV wrapper valid flag is cleared
     * unless exception is raised.
     *
     * @param[in] responseHandler The TlvResponseHandlerInterface which describe the handled TLV
     * @param[in] buffer The TLV list buffer
     * @throw TlvUnpack::Exception
     */
    TlvUnpack(const TlvResponseHandlerInterface &responseHandler, const util::Buffer &buffer);

    /**
     * Read the next value in the TLV buffer. This method should be called until it returns false.
     * Once it has returned false, nothing more can be read and all valid flag of read values are
     * set. Even if the methods raises an exception, it can and shall be called again while it
     * returns true.
     * @return true if there might be another value to be read, false if nothing more to be read
     * @throw TlvUnpack::Exception
     */
    bool readNext();

private:
    const TlvResponseHandlerInterface &mResponseHandler;
    util::MemoryByteStreamReader mBufferReader;
};
}
}
