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
