/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "Util/Stream.hpp"

namespace debug_agent
{
namespace util
{
/** Ring Buffer output stream class is a stream with a limited size, for which write operation
 * may blocks until the requested amount of data has been written.
 * Wait / getAvailable allow to perform non blocking operation on this stream.
 */
class RingBufferOutputStream : public OutputStream
{
public:
    /**
     * As write operation may block, it allows the client to stop the stream. By closing it,
     * it shall unblock the ongoing write operation.
     */
    virtual void close() = 0;

    /**
     * As write operation may block, it allows the client to wait until data can be written
     * to the ring buffer.
     * it shall be unblocked upon close operation.
     *
     * @return true if wait is successful, ie data can now be safely written (i.e. without
     * blocking), false if failed to wait (the stream may have closed voluntarily or encountered
     * an error).
     */
    virtual bool wait() = 0;

    /**
     * As write operation may block, it allows the client to check the amount of data that
     * can be written to the ring buffer.
     * @return available free space within the ring buffer.
     */
    virtual std::size_t getAvailable() = 0;

    /**
     * @return the size of the ring buffer output stream.
     */
    virtual std::size_t getSize() const = 0;
};
}
}
