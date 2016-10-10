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

#include "cAVS/Linux/MockedControlDevice.hpp"
#include "cAVS/Linux/Logger.hpp"
#include "Util/Buffer.hpp"

namespace debug_agent
{
namespace cavs
{
namespace linux
{

class MockedControlDeviceCommands final
{
public:
    MockedControlDeviceCommands(MockedControlDevice &device) : mControlDevice(device) {}

    void addGetLogLevelCommand(bool controlSuccess, mixer_ctl::LogPriority expectedLogPrio);

    void addSetLogLevelCommand(bool controlSuccess, mixer_ctl::LogPriority logPrio);

    void addGetProbeControlCommand(bool controlSuccess, unsigned int probeIndex,
                                   mixer_ctl::ProbeControl expectedProbeControl);

    void addSetProbeInjectControlCommand(bool controlSuccess, unsigned int probeIndex,
                                         mixer_ctl::ProbeControl probeControl);

    void addSetProbeExtractControlCommand(bool controlSuccess, unsigned int probeIndex,
                                          mixer_ctl::ProbeControl probeControl);

private:
    MockedControlDeviceCommands(const MockedControlDeviceCommands &) = delete;
    MockedControlDeviceCommands &operator=(const MockedControlDeviceCommands &) = delete;

    MockedControlDevice &mControlDevice;
};
}
}
}
