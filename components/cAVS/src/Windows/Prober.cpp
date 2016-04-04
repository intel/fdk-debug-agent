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

#include "cAVS/Windows/Prober.hpp"
#include "cAVS/Windows/IoctlHelpers.hpp"
#include "Util/AssertAlways.hpp"
#include <functional>
#include <string>
#include <cstring>
#include <map>
#include <algorithm>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

static const std::map<Prober::ProbePurpose, driver::ProbePurpose> purposeConversion = {
    {Prober::ProbePurpose::Inject, driver::ProbePurpose::InjectPurpose},
    {Prober::ProbePurpose::Extract, driver::ProbePurpose::ExtractPurpose},
    {Prober::ProbePurpose::InjectReextract, driver::ProbePurpose::InjectReextractPurpose}};

static const std::map<Prober::State, driver::ProbeState> stateConversion = {
    {Prober::State::Idle, driver::ProbeState::ProbeFeatureIdle},
    {Prober::State::Owned, driver::ProbeState::ProbeFeatureOwned},
    {Prober::State::Allocated, driver::ProbeState::ProbeFeatureAllocated},
    {Prober::State::Active, driver::ProbeState::ProbeFeatureActive}};

driver::ProbeState Prober::toWindows(const State &from)
{
    try {
        return stateConversion.at(from);
    } catch (std::out_of_range &) {
        throw Exception("Wrong state value (" + std::to_string(static_cast<uint32_t>(from)) + ").");
    }
}

Prober::State Prober::fromWindows(const driver::ProbeState &from)
{
    for (auto &candidate : stateConversion) {
        if (candidate.second == from) {
            return candidate.first;
        }
    }
    throw Exception("Wrong state value (" + std::to_string(static_cast<uint32_t>(from)) + ").");
}

driver::ProbePurpose Prober::toWindows(const ProbePurpose &from)
{
    try {
        return purposeConversion.at(from);
    } catch (std::out_of_range &) {
        throw Exception("Wrong purpose value (" + std::to_string(static_cast<uint32_t>(from)) +
                        ").");
    }
}

Prober::ProbePurpose Prober::fromWindows(const driver::ProbePurpose &from)
{
    for (auto &candidate : purposeConversion) {
        if (candidate.second == from) {
            return candidate.first;
        }
    }
    throw Exception("Wrong purpose value (" + std::to_string(static_cast<uint32_t>(from)) + ").");
}

BOOL Prober::toWindows(bool value)
{
    return value ? TRUE : FALSE;
}

bool Prober::fromWindows(BOOL value)
{
    switch (value) {
    case TRUE:
        return true;
    case FALSE:
        return false;
    }
    throw Exception("Unknown BOOL value: " + std::to_string(value));
}

Prober::Prober(Device &device, const EventHandles &eventHandles)
    : mDevice(device), mEventHandles(eventHandles)
{
    for (std::size_t probeIndex = 0; probeIndex < getMaxProbeCount(); ++probeIndex) {
        mExtractionQueues.emplace_back(mQueueSize,
                                       [](const util::Buffer &buffer) { return buffer.size(); });
    }
}

void Prober::setState(State state)
{
    switch (state) {
    case State::Active:
        startStreaming();
        break;
    // If not active, stop streaming
    case State::Idle:
    case State::Owned:
    case State::Allocated:
        stopStreaming();
    }

    auto tmp = toWindows(state);
    ioctl<SetState>(tmp);
}

Prober::State Prober::getState()
{
    // Initialize with an illegal value in case the driver fails to overwrite it.
    driver::ProbeState state{static_cast<driver::ProbeState>(-1)};
    ioctl<GetState>(state);

    return fromWindows(state);
}

driver::ProbePointConfiguration Prober::toWindows(const cavs::Prober::SessionProbes &probes,
                                                  const windows::Prober::EventHandles &eventHandles)
{
    if (probes.size() != driver::maxProbes) {
        throw Exception("Expected to receive " + std::to_string(driver::maxProbes) +
                        " probe configurations to set. (Actually received " +
                        std::to_string(probes.size()));
    }

    // Translate the high-level data into driver-specific structures
    std::vector<driver::ProbePointConnection> connections;
    std::size_t probeIndex = 0;
    for (const auto &probe : probes) {
        connections.emplace_back(toWindows(probe.enabled), probe.probePoint,
                                 toWindows(probe.purpose),
                                 eventHandles.injectionHandles[probeIndex].get());

        ++probeIndex;
    }

    driver::ProbePointConfiguration toDriver;
    toDriver.extractionBufferCompletionEventHandle = eventHandles.extractionHandle->handle();
    std::copy_n(connections.begin(), driver::maxProbes, toDriver.probePointConnection);
    return toDriver;
}

void Prober::setSessionProbes(const SessionProbes probes)
{
    mCachedProbeConfiguration = probes;
    ioctl<SetProbePointConfiguration>(toWindows(probes, mEventHandles));
}

Prober::SessionProbes Prober::getSessionProbes()
{
    driver::ProbePointConfiguration from;
    memset(&from, 0xFF, sizeof(from));
    ioctl<GetProbePointConfiguration>(from);

    SessionProbes result;
    for (const auto &connection : from.probePointConnection) {
        result.emplace_back(fromWindows(connection.enabled), connection.probePointId,
                            fromWindows(connection.purpose));
    }
    return result;
}

void Prober::checkProbeId(ProbeId probeId) const
{
    if (probeId.getValue() >= getMaxProbeCount()) {
        throw Exception("Invalid probe index: " + std::to_string(probeId.getValue()));
    }
}

std::unique_ptr<util::Buffer> Prober::dequeueExtractionBlock(ProbeId probeIndex)
{
    checkProbeId(probeIndex);
    return mExtractionQueues[probeIndex.getValue()].remove();
}

bool Prober::enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer)
{
    /* TO DO */
    return false;
}

driver::RingBuffersDescription Prober::getRingBuffers()
{
    using RingBuffer = driver::RingBufferDescription;
    using RingBuffers = driver::RingBuffersDescription;

    driver::RingBuffersDescription from;
    memset(&from, 0xFF, sizeof(from));
    ioctl<GetRingBuffersDescription>(from);

    return from;
}

size_t Prober::getExtractionRingBufferLinearPosition()
{
    uint64_t from;
    memset(&from, 0xFF, sizeof(from));
    ioctl<GetExtractionRingBufferPosition>(from);
    return size_t{from};
}

template <class T>
void Prober::ioctl(typename T::Data &inout)
{
    static_assert(T::type == driver::IoCtlType::TinyGet || T::type == driver::IoCtlType::TinySet,
                  "For now, windows::Prober::ioctl only supports Tiny ioctls");
    using driver::to_string;
    using std::to_string;

    /* Creating the body payload using the IoctlFwLogsState type */
    util::MemoryByteStreamWriter bodyPayloadWriter;
    bodyPayloadWriter.write(inout);

    /* Creating the TinySet/Get ioctl buffer */
    util::Buffer buffer =
        IoctlHelpers::toTinyCmdBuffer(T::feature, T::id, bodyPayloadWriter.getBuffer());

    try {
        mDevice.ioControl(T::type, &buffer, &buffer);
    } catch (Device::Exception &e) {
        throw Exception(to_string(T::type) + " error: " + e.what());
    }

    NTSTATUS driverStatus;
    util::Buffer bodyPayloadBuffer;
    /* Parsing returned buffer */
    std::tie(driverStatus, bodyPayloadBuffer) = IoctlHelpers::fromTinyCmdBuffer(buffer);

    if (!NT_SUCCESS(driverStatus)) {
        throw Exception("Driver returns invalid status: " +
                        std::to_string(static_cast<uint32_t>(driverStatus)));
    }

    try {
        /* Reading structure from body payload */
        util::MemoryByteStreamReader reader(bodyPayloadBuffer);
        reader.read(inout);

        if (!reader.isEOS()) {
            throw Exception("Probe ioctl buffer has not been fully consumed, type=" +
                            to_string(T::type) + ", pointer=" +
                            to_string(reader.getPointerOffset()) + ", size=" +
                            to_string(reader.getBuffer().size()) + ", remaining=" +
                            to_string(reader.getBuffer().size() - reader.getBuffer().size()));
        }
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Cannot decode probe parameter ioctl buffer: " + std::string(e.what()));
    }
}

std::pair<std::set<ProbeId> /*Extract*/, std::set<ProbeId> /*Inject*/> Prober::getActiveProbes()
    const
{
    std::set<ProbeId> extractionProbes;
    std::set<ProbeId> injectionProbes;

    ProbeId::RawType probeIndex{0};
    for (auto &probe : mCachedProbeConfiguration) {
        if (probe.enabled) {
            ASSERT_ALWAYS(probePurposeHelper().isValid(probe.purpose));
            if (probe.purpose == ProbePurpose::Extract ||
                probe.purpose == ProbePurpose::InjectReextract) {
                extractionProbes.insert(ProbeId{probeIndex});
            }
            if (probe.purpose == ProbePurpose::Inject ||
                probe.purpose == ProbePurpose::InjectReextract) {
                injectionProbes.insert(ProbeId{probeIndex});
            }
        }
        ++probeIndex;
    }
    return {extractionProbes, injectionProbes};
}

void Prober::startStreaming()
{
    auto ringBuffers = getRingBuffers();

    auto result = getActiveProbes();
    auto &extractionProbes = result.first;

    try {
        if (!extractionProbes.empty()) {

            // opening queues of the active probes, and collecting probe point id
            probe::Extractor::ProbePointMap probePointMap;
            for (auto probeId : extractionProbes) {
                mExtractionQueues[probeId.getValue()].open();

                dsp_fw::ProbePointId probePointId =
                    mCachedProbeConfiguration[probeId.getValue()].probePoint;
                // Checking that the probe point id is not already in the map
                if (probePointMap.find(probePointId) != probePointMap.end()) {
                    throw Exception("Two active extraction probes have the same probe point id: " +
                                    probePointId.toString());
                }

                probePointMap[probePointId] = probeId;
            }

            // starting exractor
            mExtractor = std::make_unique<probe::Extractor>(
                *mEventHandles.extractionHandle,
                util::RingBufferReader(ringBuffers.extractionRBDescription.startAdress,
                                       ringBuffers.extractionRBDescription.size,
                                       [this] { return getExtractionRingBufferLinearPosition(); }),
                probePointMap, mExtractionQueues);
        }
    } catch (std::exception &e1) {
        try {
            std::cerr << "Cancelling starting because: " << e1.what() << std::endl;
            stopStreaming();
        } catch (Exception &e2) {
            std::cerr << "Unable to cancel starting : " << e2.what() << std::endl;
        }
    }
}

void Prober::stopStreaming()
{
    // Deleting extractor (the packet producer)
    mExtractor.reset();

    // then closing the queues to make wakup listening threads
    for (std::size_t probeIndex = 0; probeIndex < getMaxProbeCount(); ++probeIndex) {
        mExtractionQueues[probeIndex].close();
    }
}
}
}
}
