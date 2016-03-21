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
    auto &&probeEvents = windows::Prober::SystemEventHandlesFactory::createHandles();
    MockedDeviceCommands commands(device);
    Me prober(device, probeEvents);

    // Check that all states are correctly converted (special case for `Active`
    // because of a side-effect.
    for (auto state :
         {driver::ProbeState::Idle, driver::ProbeState::Owned, driver::ProbeState::Allocated}) {
        commands.addSetProbeStateCommand(true, STATUS_SUCCESS, state);
        commands.addGetProbeStateCommand(true, STATUS_SUCCESS, state);
    }

    for (auto state : {Me::State::Idle, Me::State::Owned, Me::State::Allocated}) {
        CHECK_NOTHROW(prober.setState(state));
        CHECK(prober.getState() == state);
    }

    // Setting the state to `Active` triggers an ioctl to retrieve the ring
    // buffers descriptions.
    windows::driver::RingBuffersDescription rb = {{nullptr, 0}, {}};
    commands.addGetRingBuffers(true, STATUS_SUCCESS, rb);
    commands.addSetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::Active);
    CHECK_NOTHROW(prober.setState(Me::State::Active));

    commands.addGetProbeStateCommand(true, STATUS_SUCCESS, driver::ProbeState::Active);
    CHECK(prober.getState() == Me::State::Active);

    // Have the prober throw on an illegal state (arguments passed to
    // addSetProbeStateCommand are irrelevant)
    CHECK_THROWS_AS_MSG(prober.setState(static_cast<Me::State>(42)), Me::Exception,
                        "Wrong state value (42).")

    // Have the driver return an error (the ProbeState argument is irrelevant)
    commands.addSetProbeStateCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO, driver::ProbeState::Idle);
    CHECK_THROWS_AS_MSG(prober.setState(Me::State::Idle), Me::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));

    // Have the OS make the ioctl() call fail (driver status and ProbeState
    // arguments are irrelevant)
    commands.addSetProbeStateCommand(false, STATUS_SUCCESS, driver::ProbeState::Idle);
    CHECK_THROWS_AS_MSG(prober.setState(Me::State::Idle), Me::Exception,
                        "TinySet error: OS says that io control has failed.");
}
TEST_CASE_METHOD(Fixture, "Probing: set/getSessionProbes", "[prober]")
{
    auto &&probeEvents = windows::Prober::SystemEventHandlesFactory::createHandles();
    MockedDeviceCommands commands(device);
    Me prober(device, probeEvents);

    Me::SessionProbes sampleCavsConfig = {
        {true, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Inject},
        {true, {4, 3, Me::ProbeType::Internal, 1}, Me::ProbePurpose::Extract},
        {true, {0xffff, 0xff, Me::ProbeType::Internal, 0x3f}, Me::ProbePurpose::InjectReextract},
        {false, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Inject},
        {true, {0, 0, Me::ProbeType::Output, 0}, Me::ProbePurpose::Extract},
        {false, {16, 8, Me::ProbeType::Internal, 4}, Me::ProbePurpose::Extract},
        {true, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Extract},
        {true, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Extract}};

    driver::ProbePointConfiguration sampleDriverConfig =
        Me::toWindows(sampleCavsConfig, probeEvents);

    // Check nominal cases
    commands.addSetProbeConfigurationCommand(true, STATUS_SUCCESS, sampleDriverConfig);
    CHECK_NOTHROW(prober.setSessionProbes(sampleCavsConfig));
    commands.addGetProbeConfigurationCommand(true, STATUS_SUCCESS, sampleDriverConfig);
    CHECK_NOTHROW(prober.getSessionProbes() == sampleCavsConfig);

    // Check that illegal values are properly detected
    std::vector<Me::SessionProbes> illegalCavsConfigs;
    uint32_t illegalModuleId = 1 << driver::ProbePointId::moduleIdSize;
    uint32_t illegalInstanceId = 1 << driver::ProbePointId::instanceIdSize;
    Me::ProbeType illegalType = static_cast<Me::ProbeType>(1 << driver::ProbePointId::typeSize);
    uint32_t illegalIndex = 1 << driver::ProbePointId::indexSize;

    for (auto illegal : {Me::ProbePointId{illegalModuleId, 0, Me::ProbeType::Input, 0},
                         Me::ProbePointId{0, illegalInstanceId, Me::ProbeType::Input, 0},
                         Me::ProbePointId{0, 0, illegalType, 0},
                         Me::ProbePointId{0, 0, Me::ProbeType::Input, illegalIndex}}) {
        Me::SessionProbes illegalCavsConfig{{true, illegal, Me::ProbePurpose::Inject}};
        // The rest of the vector is just here as mandatory padding
        illegalCavsConfig.insert(end(illegalCavsConfig), 7,
                                 {true, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Inject});

        illegalCavsConfigs.emplace_back(illegalCavsConfig);
    }

    using std::to_string;
    CHECK_THROWS_AS_MSG(prober.setSessionProbes(illegalCavsConfigs[0]), Me::Exception,
                        "Module id too large (" + to_string(illegalModuleId) + ").");
    CHECK_THROWS_AS_MSG(prober.setSessionProbes(illegalCavsConfigs[1]), Me::Exception,
                        "Instance id too large (" + to_string(illegalInstanceId) + ").");
    CHECK_THROWS_AS_MSG(prober.setSessionProbes(illegalCavsConfigs[2]), Me::Exception,
                        "Type too large (" + to_string(static_cast<uint32_t>(illegalType)) + ").");
    CHECK_THROWS_AS_MSG(prober.setSessionProbes(illegalCavsConfigs[3]), Me::Exception,
                        "Pin index too large (" + to_string(illegalIndex) + ").");

    Me::SessionProbes illegalPurposeCavsConfig{
        {true, {0, 0, Me::ProbeType::Input, 0}, static_cast<Me::ProbePurpose>(3)}};
    // The rest of the vector is just here as mandatory padding
    illegalPurposeCavsConfig.insert(
        end(illegalPurposeCavsConfig), 7,
        {true, {0, 0, Me::ProbeType::Input, 0}, Me::ProbePurpose::Inject});
    CHECK_THROWS_AS_MSG(prober.setSessionProbes(illegalPurposeCavsConfig), Me::Exception,
                        "Wrong purpose value (3).");

    // Have the driver return an error (the ProbeState argument is irrelevant)
    commands.addSetProbeConfigurationCommand(true, STATUS_FLOAT_DIVIDE_BY_ZERO, sampleDriverConfig);
    CHECK_THROWS_AS_MSG(prober.setSessionProbes(sampleCavsConfig), Me::Exception,
                        "Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(STATUS_FLOAT_DIVIDE_BY_ZERO)));

    // Have the OS make the ioctl() call fail (driver status and ProbeState
    // arguments are irrelevant)
    commands.addSetProbeConfigurationCommand(false, STATUS_SUCCESS, sampleDriverConfig);
    CHECK_THROWS_AS_MSG(prober.setSessionProbes(sampleCavsConfig), Me::Exception,
                        "TinySet error: OS says that io control has failed.");
}
