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
#include "cAVS/Linux/ControlDeviceTypes.hpp"
#include "Util/Buffer.hpp"
#include "Util/ByteStreamWriter.hpp"
#include <map>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

class StubbedControlDevice final : public ControlDevice
{
public:
    /** @throw Device::Exception if the device initialization has failed */
    StubbedControlDevice(const std::string &name) : ControlDevice(name)
    {
        util::MemoryByteStreamWriter writer;
        writer.write(static_cast<long>(mixer_ctl::LogPriority::Critical));
        mBackupedControl[mixer_ctl::logLevelMixer] = writer.getBuffer();
    }

    void ctlRead(const std::string &name, util::Buffer &bufferOutput) override
    {
        if (mBackupedControl.find(name) != mBackupedControl.end()) {
            bufferOutput = mBackupedControl[name];
            std::cout << "CtlRead for " << name << " found" << std::endl;
        } else
            std::cout << "CtlRead for " << name << " NOT found" << std::endl;
    }

    void ctlWrite(const std::string &name, const util::Buffer &bufferInput) override
    {
        mBackupedControl[name] = bufferInput;
    }

    size_t getControlCountByTag(const std::string & /*tag*/) const override
    {
        /** used to get max probe count in extraction and in injection.
         * So answer 4 (4 for extract, 4 for injection to get 8 probe point...). */
        return 4;
    }

private:
    std::map<std::string, util::Buffer> mBackupedControl;
};
}
}
}
