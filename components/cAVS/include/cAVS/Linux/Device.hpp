/*
 * Copyright (c) 2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Util/Buffer.hpp"
#include <exception>
#include <string>
#include <inttypes.h>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class abstracts a Linux Debug file system (open,close,read,write...)
 */
class Device
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** @throw Device::Exception if the device initialization has failed */
    Device() = default;
    virtual ~Device() = default;

    /** Write a command to the debugFs entry "name" with the given input buffer.
     *
     * @param[in] name of the debug fs entry.
     * @param[in] bufferInput command to write.
     *
     * @return bytes writen to the debugfs entry
     * @throw Device::Exception in case of write error.
     */
    virtual ssize_t commandWrite(const std::string &name, const util::Buffer &bufferInput) = 0;

    /** read a command to the debugFs entry "name" with the given input buffer as a command
     * and store the result of the read command in the output buffer.
     *
     * @param[in] name of the debug fs entry.
     * @param[in] bufferInput command to write.
     * @param[out] bufferOutput result of the read command.
     *
     * @throw Device::Exception in case of read error.
     */
    virtual void commandRead(const std::string &name, const util::Buffer &bufferInput,
                             util::Buffer &bufferOutput) = 0;

private:
    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;
};
}
}
}
