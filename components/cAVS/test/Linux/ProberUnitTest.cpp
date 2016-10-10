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

#include "TestCommon/TestHelpers.hpp"
#include "cAVS/Linux/MockedDevice.hpp"
#include "cAVS/Linux/StubbedControlDevice.hpp"
#include "cAVS/Linux/MockedControlDeviceCommands.hpp"
#include "cAVS/Linux/MockedDeviceCatchHelper.hpp"
#include "cAVS/Linux/StubbedCompressDeviceFactory.hpp"
#include "cAVS/Linux/MockedDeviceCommands.hpp"
#include "cAVS/Linux/Prober.hpp"
#include <catch.hpp>
#include <memory>

using namespace debug_agent::cavs;
using namespace debug_agent::cavs::linux;

using Fixture = MockedDeviceFixture;
using LinuxProber = linux::Prober;

TEST_CASE_METHOD(Fixture, "Probing: set/getState", "[prober]")
{
    StubbedCompressDeviceFactory compressDeviceFactory;
    std::unique_ptr<ControlDevice> stubbedControlDevice =
        std::make_unique<StubbedControlDevice>("myStubbedControlCard");

    LinuxProber prober(*stubbedControlDevice, compressDeviceFactory);

    CHECK_NOTHROW(prober.setState(true));

    CHECK(prober.isActive() == true);
}

TEST_CASE_METHOD(Fixture, "Probing: set/getSessionProbes", "[prober]")
{
    StubbedCompressDeviceFactory compressDeviceFactory;
    MockedControlDeviceCommands commands(*controlDevice);

    LinuxProber prober(*controlDevice, compressDeviceFactory);

    const LinuxProber::SessionProbes extractionProbeConfiguration(
        {{true, {4, 3, dsp_fw::ProbeType::Internal, 1}, LinuxProber::ProbePurpose::Extract},
         {true,
          {0xffff, 0xff, dsp_fw::ProbeType::Internal, 0x3f},
          LinuxProber::ProbePurpose::InjectReextract},
         {true, {0, 0, dsp_fw::ProbeType::Output, 0}, LinuxProber::ProbePurpose::Extract}});

    /** For inactive injection probe, no control mixer will be set as the compress device associated
     * will not be opened, so no IPC command to the FW to be sent
     */
    const LinuxProber::SessionProbes injectionProbeConfiguration(
        {{true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
         {true,
          {0xffff, 0xff, dsp_fw::ProbeType::Internal, 0x3f},
          LinuxProber::ProbePurpose::InjectReextract},
         {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
         {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject}});

    const LinuxProber::SessionProbes sampleCavsConfig(
        {{true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
         {true, {4, 3, dsp_fw::ProbeType::Internal, 1}, LinuxProber::ProbePurpose::Extract},
         {true,
          {0xffff, 0xff, dsp_fw::ProbeType::Internal, 0x3f},
          LinuxProber::ProbePurpose::InjectReextract},
         {false, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
         {true, {0, 0, dsp_fw::ProbeType::Output, 0}, LinuxProber::ProbePurpose::Extract},
         {false, {16, 8, dsp_fw::ProbeType::Internal, 4}, LinuxProber::ProbePurpose::Extract},
         {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
         {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject}});

    /* Currently not used */
    LinuxProber::InjectionSampleByteSizes injectionSampleByteSizes;

    // Check nominal cases
    std::size_t probeIndex = 0;
    for (const auto &sessionProbe : extractionProbeConfiguration) {
        mixer_ctl::ProbeControl probeControl(LinuxProber::toLinux(sessionProbe));
        commands.addSetProbeExtractControlCommand(true, probeIndex, probeControl);
        ++probeIndex;
    }

    probeIndex = 0;
    for (const auto &sessionProbe : injectionProbeConfiguration) {
        mixer_ctl::ProbeControl probeControl(LinuxProber::toLinux(sessionProbe));
        commands.addSetProbeInjectControlCommand(true, probeIndex, probeControl);
        ++probeIndex;
    }

    CHECK_NOTHROW(prober.setProbesConfig(sampleCavsConfig, injectionSampleByteSizes));
    CHECK_NOTHROW(prober.getProbesConfig() == sampleCavsConfig);

    LinuxProber::SessionProbes illegalPurposeCavsConfig(
        {{true, {0, 0, dsp_fw::ProbeType::Input, 0}, static_cast<LinuxProber::ProbePurpose>(3)}});
    // The rest of the vector is just here as mandatory padding
    illegalPurposeCavsConfig.insert(
        end(illegalPurposeCavsConfig), 7,
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject});
    CHECK_THROWS_AS_MSG(prober.setProbesConfig(illegalPurposeCavsConfig, injectionSampleByteSizes),
                        LinuxProber::Exception, "Wrong purpose value (3).");

    // Have the control device failing to set the params
    mixer_ctl::ProbeControl probeControl(LinuxProber::toLinux(extractionProbeConfiguration[0]));
    commands.addSetProbeExtractControlCommand(false, 0, probeControl);

    CHECK_THROWS_AS_MSG(prober.setProbesConfig(sampleCavsConfig, injectionSampleByteSizes),
                        LinuxProber::Exception,
                        "Failed to write extration probe control settings: "
                        "Control Device says that control Write has failed.");
}
