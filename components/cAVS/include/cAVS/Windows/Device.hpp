/*
 * Copyright (c) 2015, Intel Corporation
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

#include "cAVS/Windows/DriverTypes.hpp"
#include "Util/Buffer.hpp"
#include <exception>
#include <string>
#include <inttypes.h>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class abstracts a Windows device that supports io control.
 * The ioctl API wraps the windows DeviceIoControl function, see:
 * https://msdn.microsoft.com/en-us/library/windows/desktop/aa363216(v=vs.85).aspx
 */
class Device
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** @throw Device::Exception if the device initialization has failed */
    Device() {}
    virtual ~Device() {}

    /**
     * Perform an IO control
     * @param[in] ioControlType the IO control type
     * @param[in] input the input buffer. This parameter is optional, use nullptr if the
     *                        input buffer is not required.
     * @param[in,out] output the output buffer (basically this buffer can be used as input
     *                             buffer too). This parameter is optional, use nullptr if the
     *                             output buffer is not required. The output Buffer is resized
     *                             according to the IO control response.
     * @throw Device::Exception if the io control has failed
     */
    void ioControl(driver::IoCtlType ioControlType, const util::Buffer *input, util::Buffer *output)
    {
        ioControl(static_cast<uint32_t>(ioControlType), input, output);
    }
    /** Perform an IO control
     *
     * @see ioControl. This overload uses an direct integer value instead of an
     * enum.
     *
     * @todo: Move Windows/Device out of cAVS, so that it won't have to implement
     * this overload. Or templatize the Device class with an ioctl type?
     */
    virtual void ioControl(uint32_t ioControlCode, const util::Buffer *input,
                           util::Buffer *output) = 0;

private:
    Device(const Device &) = delete;
    Device &operator=(const Device &) = delete;
};
}
}
}
