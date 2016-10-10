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
