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

#include "Util/Buffer.hpp"
#include <exception>
#include <string>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class abstracts a Linux ALSA Device for control operation (ctl read, ctl write)
 */
class ControlDevice
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** @throw Device::Exception if the device initialization has failed */
    ControlDevice(const std::string &name) : mName(name) {}
    virtual ~ControlDevice() = default;

    virtual void ctlRead(const std::string &name, util::Buffer &bufferOutput) = 0;
    virtual void ctlWrite(const std::string &name, const util::Buffer &bufferInput) = 0;

    const std::string getCardName() const { return mName; }

private:
    ControlDevice(const ControlDevice &) = delete;
    ControlDevice &operator=(const ControlDevice &) = delete;

    std::string mName; /** ALSA Device name. */
};
}
}
}
