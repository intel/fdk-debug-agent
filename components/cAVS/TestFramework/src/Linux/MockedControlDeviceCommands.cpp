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

#include "cAVS/Linux/MockedControlDeviceCommands.hpp"
#include "cAVS/Linux/DriverTypes.hpp"
#include "Util/Buffer.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/ByteStreamReader.hpp"

using namespace debug_agent::util;

namespace debug_agent
{
namespace cavs
{
namespace linux
{

void MockedControlDeviceCommands::addGetLogLevelCommand(bool controlSuccess,
                                                        mixer_ctl::LogPriority expectedLogPrio)
{
    MemoryByteStreamWriter writer;
    writer.write(expectedLogPrio);
    if (controlSuccess) {
        mControlDevice.addSuccessfulControlReadEntry(mixer_ctl::logLevelMixer, {},
                                                     writer.getBuffer());
    } else {
        mControlDevice.addFailedControlReadEntry(mixer_ctl::logLevelMixer, {}, writer.getBuffer());
    }
}

void MockedControlDeviceCommands::addSetLogLevelCommand(bool controlSuccess,
                                                        mixer_ctl::LogPriority logPrio)
{
    MemoryByteStreamWriter writer;
    writer.write(logPrio);
    mControlDevice.addControlWriteEntry(controlSuccess, mixer_ctl::logLevelMixer,
                                        writer.getBuffer());
}

void MockedControlDeviceCommands::addGetProbeControlCommand(
    bool controlSuccess, unsigned int probeIndex, mixer_ctl::ProbeControl expectedProbeControl)
{
    MemoryByteStreamWriter writer;
    writer.write(expectedProbeControl);
    if (expectedProbeControl.getPurpose() == mixer_ctl::ProbePurpose::Inject ||
        expectedProbeControl.getPurpose() == mixer_ctl::ProbePurpose::InjectReextract) {
        mControlDevice.addControlReadEntry(
            controlSuccess, mixer_ctl::getProbeInjectControl(probeIndex), {}, writer.getBuffer());
    } else {
        mControlDevice.addControlReadEntry(
            controlSuccess, mixer_ctl::getProbeExtractControl(probeIndex), {}, writer.getBuffer());
    }
}

void MockedControlDeviceCommands::addSetProbeInjectControlCommand(
    bool controlSuccess, unsigned int probeIndex, mixer_ctl::ProbeControl probeControl)
{
    MemoryByteStreamWriter writer;
    writer.write(probeControl);
    mControlDevice.addControlWriteEntry(
        controlSuccess, mixer_ctl::getProbeInjectControl(probeIndex), writer.getBuffer());
}
void MockedControlDeviceCommands::addSetProbeExtractControlCommand(
    bool controlSuccess, unsigned int probeIndex, mixer_ctl::ProbeControl probeControl)
{
    MemoryByteStreamWriter writer;
    writer.write(probeControl);
    mControlDevice.addControlWriteEntry(
        controlSuccess, mixer_ctl::getProbeExtractControl(probeIndex), writer.getBuffer());
}
}
}
}
