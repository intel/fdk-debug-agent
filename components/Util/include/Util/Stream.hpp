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

#include "Util/Exception.hpp"
#include <cstdint>
#include <cstddef>

namespace debug_agent
{
namespace util
{

using StreamByte = uint8_t;

/** Base input stream class */
class InputStream
{
public:
    using Exception = util::Exception<InputStream>;

    InputStream() = default;
    InputStream(const InputStream &) = delete;
    InputStream &operator=(const InputStream &) = delete;
    virtual ~InputStream() = default;

    /**
     * An input stream read function is expected to block until the required bytes have been read.
     * In some case, the client of the input stream may decide to stop the ongoing read and close
     * the stream. So, upon close, if the read is blocked, it shall unblock it.
     */
    virtual void close() {}

    /**
     * This method blocks until all the required bytes are read.
     * @return the read byte count. If it is less than byteCount, end of
     *         stream is reached.
     * @throw InputStream::Exception */
    virtual std::size_t read(StreamByte *dest, std::size_t byteCount) = 0;
};

/** Base output stream class */
class OutputStream
{
public:
    using Exception = util::Exception<OutputStream>;

    OutputStream() = default;
    OutputStream(const OutputStream &) = delete;
    OutputStream &operator=(const OutputStream &) = delete;
    virtual ~OutputStream() = default;

    /** @throw OutputStream::Exception */
    virtual void write(const StreamByte *src, std::size_t byteCount) = 0;
};
}
}
