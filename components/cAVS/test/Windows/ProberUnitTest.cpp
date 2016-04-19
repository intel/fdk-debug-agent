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
#include "cAVS/Windows/MockedDevice.hpp"
#include "cAVS/Windows/MockedDeviceCatchHelper.hpp"
#include "cAVS/Windows/MockedDeviceCommands.hpp"
#include "cAVS/Windows/EventHandle.hpp"
#include "cAVS/Windows/Prober.hpp"
#include <catch.hpp>

using namespace debug_agent::cavs;
using namespace debug_agent::cavs::windows;

using Fixture = MockedDeviceFixture;
using WinProber = windows::Prober;

TEST_CASE_METHOD(Fixture, "Probing: set/getState", "[prober]")
{
    auto &&probeEvents = windows::ProberBackend::SystemEventHandlesFactory::createHandles();
    MockedDeviceCommands commands(device);
    ProberBackend prober(device, probeEvents);

    // Check that all states are correctly converted (special case for `Active`
    // because of a side-effect.
    for (auto state : {driver::ProbeState::ProbeFeatureIdle, driver::ProbeState::ProbeFeatureOwned,
                       driver::ProbeState::ProbeFeatureAllocated}) {
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, state);
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, state);
    }

    for (auto state : {ProberBackend::State::Idle, ProberBackend::State::Owned,
                       ProberBackend::State::Allocated}) {
        CHECK_NOTHROW(prober.setStateTransition(state, state));
        CHECK(prober.getState() == state);
    }

    // Setting the state to `Active` triggers an ioctl to retrieve the ring
    // buffers descriptions.
    windows::driver::RingBuffersDescription rb = {{nullptr, 0}, {}};
    commands.addGetRingBuffers(true, STATUS_SUCCESS, rb);
    commands.addSetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureActive);
    CHECK_NOTHROW(
        prober.setStateTransition(ProberBackend::State::Active, ProberBackend::State::Active));

    commands.addGetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureActive);
    CHECK(prober.getState() == ProberBackend::State::Active);

    // Setting the state transtion from Idle to Owner triggers an ioctl to set the probes
    // configuration.
    WinProber::SessionProbes sampleCavsConfig = {
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Inject},
        {true, {4, 3, dsp_fw::ProbeType::Internal, 1}, WinProber::ProbePurpose::Extract},
        {true,
         {0xffff, 0xff, dsp_fw::ProbeType::Internal, 0x3f},
         WinProber::ProbePurpose::InjectReextract},
        {false, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Inject},
        {true, {0, 0, dsp_fw::ProbeType::Output, 0}, WinProber::ProbePurpose::Extract},
        {false, {16, 8, dsp_fw::ProbeType::Internal, 4}, WinProber::ProbePurpose::Extract},
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Extract},
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Extract}};

    /* Currently not used */
    WinProber::InjectionSampleByteSizes injectionSampleByteSizes;

    driver::ProbePointConfiguration sampleDriverConfig =
        ProberBackend::toWindows(sampleCavsConfig, probeEvents);
    commands.addSetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureIdle);
    CHECK_NOTHROW(
        prober.setStateTransition(ProberBackend::State::Idle, ProberBackend::State::Idle));
    commands.addGetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureIdle);
    CHECK(prober.getState() == ProberBackend::State::Idle);

    commands.addSetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureOwned);
    commands.addSetProbeConfigurationCommand(true, STATUS_SUCCESS, sampleDriverConfig);

    CHECK_NOTHROW(prober.setSessionProbes(sampleCavsConfig, injectionSampleByteSizes));
    CHECK_NOTHROW(
        prober.setStateTransition(ProberBackend::State::Owned, ProberBackend::State::Idle));
    commands.addGetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureOwned);
    CHECK(prober.getState() == ProberBackend::State::Owned);

    // Have the prober throw on an illegal state (arguments passed to
    // addSetProbeStateCommand are irrelevant)
    CHECK_THROWS_AS_MSG(prober.setStateTransition(static_cast<ProberBackend::State>(42),
                                                  ProberBackend::State::Active),
                        ProberBackend::Exception, "Wrong state value (42).");

    // Have the driver return an error (the ProbeState argument is irrelevant)
    commands.addSetProbeStateCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO,
                                     driver::ProbeState::ProbeFeatureIdle);
    CHECK_THROWS_AS_MSG(
        prober.setStateTransition(ProberBackend::State::Idle, ProberBackend::State::Idle),
        ProberBackend::Exception,
        "Driver returns invalid status: " +
            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));

    // Have the OS make the ioctl() call fail (driver status and ProbeState
    // arguments are irrelevant)
    commands.addSetProbeStateCommand(false, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureIdle);
    CHECK_THROWS_AS_MSG(
        prober.setStateTransition(ProberBackend::State::Idle, ProberBackend::State::Idle),
        ProberBackend::Exception, "TinySet error: OS says that io control has failed.");
}
TEST_CASE_METHOD(Fixture, "Probing: set/getSessionProbes", "[prober]")
{
    auto &&probeEvents = windows::ProberBackend::SystemEventHandlesFactory::createHandles();
    MockedDeviceCommands commands(device);
    ProberBackend prober(device, probeEvents);

    WinProber::SessionProbes sampleCavsConfig = {
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Inject},
        {true, {4, 3, dsp_fw::ProbeType::Internal, 1}, WinProber::ProbePurpose::Extract},
        {true,
         {0xffff, 0xff, dsp_fw::ProbeType::Internal, 0x3f},
         WinProber::ProbePurpose::InjectReextract},
        {false, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Inject},
        {true, {0, 0, dsp_fw::ProbeType::Output, 0}, WinProber::ProbePurpose::Extract},
        {false, {16, 8, dsp_fw::ProbeType::Internal, 4}, WinProber::ProbePurpose::Extract},
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Extract},
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Extract}};

    WinProber::InjectionSampleByteSizes injectionSampleByteSizes;

    driver::ProbePointConfiguration sampleDriverConfig =
        ProberBackend::toWindows(sampleCavsConfig, probeEvents);

    // Check nominal cases
    commands.addSetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureOwned);
    commands.addSetProbeConfigurationCommand(true, STATUS_SUCCESS, sampleDriverConfig);
    CHECK_NOTHROW(prober.setSessionProbes(sampleCavsConfig, injectionSampleByteSizes));
    CHECK_NOTHROW(
        prober.setStateTransition(ProberBackend::State::Owned, ProberBackend::State::Idle));

    commands.addGetProbeConfigurationCommand(true, STATUS_SUCCESS, sampleDriverConfig);
    CHECK_NOTHROW(prober.getSessionProbes() == sampleCavsConfig);

    WinProber::SessionProbes illegalPurposeCavsConfig{
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, static_cast<WinProber::ProbePurpose>(3)}};
    // The rest of the vector is just here as mandatory padding
    illegalPurposeCavsConfig.insert(
        end(illegalPurposeCavsConfig), 7,
        {true, {0, 0, dsp_fw::ProbeType::Input, 0}, WinProber::ProbePurpose::Inject});
    commands.addSetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureOwned);
    CHECK_NOTHROW(prober.setSessionProbes(illegalPurposeCavsConfig, injectionSampleByteSizes));
    CHECK_THROWS_AS_MSG(
        prober.setStateTransition(ProberBackend::State::Owned, ProberBackend::State::Idle),
        ProberBackend::Exception, "Wrong purpose value (3).");

    // Have the driver return an error (the ProbeState argument is irrelevant)
    commands.addSetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureOwned);
    commands.addSetProbeConfigurationCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO, sampleDriverConfig);
    CHECK_NOTHROW(prober.setSessionProbes(sampleCavsConfig, injectionSampleByteSizes));
    CHECK_THROWS_AS_MSG(
        prober.setStateTransition(ProberBackend::State::Owned, ProberBackend::State::Idle),
        ProberBackend::Exception,
        "Driver returns invalid status: " +
            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));

    // Have the OS make the ioctl() call fail (driver status and ProbeState
    // arguments are irrelevant)
    commands.addSetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::ProbeFeatureOwned);
    commands.addSetProbeConfigurationCommand(false, STATUS_SUCCESS, sampleDriverConfig);
    CHECK_NOTHROW(prober.setSessionProbes(sampleCavsConfig, injectionSampleByteSizes));
    CHECK_THROWS_AS_MSG(
        prober.setStateTransition(ProberBackend::State::Owned, ProberBackend::State::Idle),
        ProberBackend::Exception, "TinySet error: OS says that io control has failed.");
}
