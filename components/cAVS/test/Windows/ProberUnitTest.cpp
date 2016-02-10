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
// Avoid conflicts between cavs::Prober and cavs::windows::Prober
using Me = windows::Prober;

TEST_CASE_METHOD(Fixture, "Probing: set/getState", "[prober]")
{
    EventHandle probeEvent;
    MockedDeviceCommands commands(device);

    // Check that all states are correctly converted
    for (auto state : {driver::ProbeState::Idle, driver::ProbeState::Owned,
                       driver::ProbeState::Allocated, driver::ProbeState::Active}) {
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, state);
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, state);
    }

    Me prober(device, probeEvent);

    for (auto state :
         {Me::State::Idle, Me::State::Owned, Me::State::Allocated, Me::State::Active}) {
        CHECK_NOTHROW(prober.setState(state));
        CHECK(prober.getState() == state);
    }

    // TODO: error cases
}
TEST_CASE_METHOD(Fixture, "Probing: set/getSessionProbes", "[prober]")
{
    EventHandle probeEvent;
    MockedDeviceCommands commands(device);

    driver::ProbePointConfiguration sampleDriverConfig = {
        probeEvent.get(),
        {{true, {0, 0, 0, 0}, driver::ProbePurpose::Inject, nullptr},
         {true, {4, 3, 2, 1}, driver::ProbePurpose::Extract, nullptr},
         {true, {0xffff, 0xff, 2, 0x3f}, driver::ProbePurpose::InjectReextract, nullptr},
         {false, {0, 0, 0, 0}, driver::ProbePurpose::Inject, nullptr},
         {true, {0, 0, 1, 0}, driver::ProbePurpose::Extract, nullptr},
         {false, {16, 8, 2, 4}, driver::ProbePurpose::Extract, nullptr},
         {true, {0, 0, 0, 0}, driver::ProbePurpose::Extract, nullptr},
         {true, {0, 0, 0, 0}, driver::ProbePurpose::Extract, nullptr}}};
    Me::SessionProbes sampleCavsConfig = {
        {true, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Inject},
        {true, {4, 3, Me::ProbeType::Internal, 1}, Me::ProbePurpose::Extract},
        {true, {0xffff, 0xff, Me::ProbeType::Internal, 0x3f}, Me::ProbePurpose::InjectReextract},
        {false, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Inject},
        {true, {0, 0, Me::ProbeType::Output, 0}, Me::ProbePurpose::Extract},
        {false, {16, 8, Me::ProbeType::Internal, 4}, Me::ProbePurpose::Extract},
        {true, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Extract},
        {true, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Extract}};

    commands.addSetProbeConfigurationCommand(true, STATUS_SUCCESS, sampleDriverConfig);
    commands.addGetProbeConfigurationCommand(true, STATUS_SUCCESS, sampleDriverConfig);

    Me prober(device, probeEvent);

    CHECK_NOTHROW(prober.setSessionProbes(sampleCavsConfig));
    CHECK_NOTHROW(prober.getSessionProbes() == sampleCavsConfig);

    // TODO: error cases
    // TODO: invalid probe point ids
}
