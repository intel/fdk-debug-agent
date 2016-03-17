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

#include "cAVS/Linux/ControlDevice.hpp"

namespace private_driver
{
#include <alsa/asoundlib.h>
}

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/** This class abstracts a Linux ALSA Device for control operation (ctl read, ctl write)
 */
class AlsaControlDevice final : public ControlDevice
{
public:
    /** @throw Device::Exception if the device initialization has failed */
    AlsaControlDevice(const std::string &name);

    void ctlRead(const std::string &name, util::Buffer &bufferOutput) override;
    void ctlWrite(const std::string &name, const util::Buffer &bufferInput) override;

private:
    void getCtlHandle(const std::string &name, private_driver::snd_ctl_t *&handle,
                      private_driver::snd_ctl_elem_id_t &id,
                      private_driver::snd_ctl_elem_info_t &info,
                      private_driver::snd_ctl_elem_value_t &control) const;

    static std::string getCtlName(const std::string &name)
    {
        return std::string{"name='" + name + "'"};
    }
    std::string mControl;
};
}
}
}
