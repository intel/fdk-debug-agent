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

    LinuxProber::SessionProbes extractionProbeConfiguration = {
        {true, {4, 3, dsp_fw::ProbeType::Internal, 1}, LinuxProber::ProbePurpose::Extract},
        {true,
         {0xffff, 0xff, dsp_fw::ProbeType::Internal, 0x3f},
         LinuxProber::ProbePurpose::InjectReextract},
        {true, {0, 0, dsp_fw::ProbeType::Output, 0}, LinuxProber::ProbePurpose::Extract},
        /* For inactive probe, the value applied to the mixer control is as followed
         * even if the cached probe configuration reflects the real probe config.
         */
        {false, {0, 0, dsp_fw::ProbeType::Output, 0}, LinuxProber::ProbePurpose::Extract}};

    /** For inactive injection probe, no control mixer will be set as the compress device associated
     * will not be opened, so no IPC command to the FW to be sent
     */
    LinuxProber::SessionProbes injectionProbeConfiguration = {
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
        {true,
         {0xffff, 0xff, dsp_fw::ProbeType::Internal, 0x3f},
         LinuxProber::ProbePurpose::InjectReextract},
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject}};

    LinuxProber::SessionProbes sampleCavsConfig = {
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
        {true, {4, 3, dsp_fw::ProbeType::Internal, 1}, LinuxProber::ProbePurpose::Extract},
        {true,
         {0xffff, 0xff, dsp_fw::ProbeType::Internal, 0x3f},
         LinuxProber::ProbePurpose::InjectReextract},
        {false, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
        {true, {0, 0, dsp_fw::ProbeType::Output, 0}, LinuxProber::ProbePurpose::Extract},
        {false, {16, 8, dsp_fw::ProbeType::Internal, 4}, LinuxProber::ProbePurpose::Extract},
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject},
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, LinuxProber::ProbePurpose::Inject}};

    /* Currently not used */
    LinuxProber::InjectionSampleByteSizes injectionSampleByteSizes;

    // Check nominal cases
    std::size_t probeIndex = 0;
    for (auto sessionProbe : extractionProbeConfiguration) {
        mixer_ctl::ProbeControl probeControl(LinuxProber::toLinux(sessionProbe));
        commands.addSetProbeExtractControlCommand(true, probeIndex, probeControl);
        ++probeIndex;
    }

    probeIndex = 0;
    for (auto sessionProbe : injectionProbeConfiguration) {
        mixer_ctl::ProbeControl probeControl(LinuxProber::toLinux(sessionProbe));
        commands.addSetProbeInjectControlCommand(true, probeIndex, probeControl);
        ++probeIndex;
    }

    CHECK_NOTHROW(prober.setProbesConfig(sampleCavsConfig, injectionSampleByteSizes));
    CHECK_NOTHROW(prober.getProbesConfig() == sampleCavsConfig);

    LinuxProber::SessionProbes illegalPurposeCavsConfig{
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, static_cast<LinuxProber::ProbePurpose>(3)}};
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
