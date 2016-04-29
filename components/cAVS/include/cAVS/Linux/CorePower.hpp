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

#include "cAVS/Linux/Device.hpp"
#include "cAVS/Linux/DriverTypes.hpp"

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/**
 * Helper class to prevent or not a core from sleeping. The device to use to send the debugfs
 * command shall be handled by the client.
 * The core concerned by the action shall be given as parameter at construction time.
 * If none is provided, the helper function will prevent all cores from sleeping.
 *
 * @tparam Exception The type of exception to be thrown.
 */
template <class Exception>
class CorePower
{
public:
    CorePower(Device &device, unsigned int coreId = allCores) : mDevice(device), mCoreId(coreId) {}

    /** Scope class that automatically prevent and allows core to sleep */
    class Auto
    {
    public:
        Auto(CorePower<Exception> &corePower) : mCorePower(corePower)
        {
            mCorePower.preventCoreFromSleeping();
        }
        ~Auto() { mCorePower.allowCoreToSleep(); }
    private:
        CorePower<Exception> &mCorePower;
    };

    /**
     * Prevent a Core from sleeping by incrementing the reference counter implemented by
     * the driver.
     */
    void preventCoreFromSleeping() { setCorePower(false); }

    /**
     * Allow a Core to sleep by decrementing the reference counter implemented by
     * the driver.
     */
    void allowCoreToSleep() { setCorePower(true); }
    void allowCoreToSleepNoExcept() noexcept
    {
        try {
            setCorePower(true);
        } catch (const Exception &e) {
            std::cerr << std::string(e.what()) << std::endl;
        }
    }
    void setCorePower(bool sleepAllowed)
    {
        driver::CorePowerCommand corePowerCmd(sleepAllowed, mCoreId);
        try {
            mDevice.commandWrite(driver::corePowerCtrl, corePowerCmd.getBuffer());
        } catch (const Device::Exception &e) {
            throw Exception("Error: could not set core power: " + std::string(e.what()));
        }
    }

private:
    /** @todo: currently: Voting for core 0 will prevent all core from sleeping.
     * Update this helper class to wake up all core when no arg is given once the debugfs command
     * will work as expected.
     */
    static const unsigned int allCores = 0;

    Device &mDevice;
    unsigned int mCoreId;
};

template <class Exception>
using AutoPreventFromSleeping = typename CorePower<Exception>::Auto;
}
}
}
