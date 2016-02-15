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
#include <string>
#include <cstring>
#include <map>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

static const std::map<Prober::ProbePurpose, driver::ProbePurpose> purposeConversion = {
    {Prober::ProbePurpose::Inject, driver::ProbePurpose::Inject},
    {Prober::ProbePurpose::Extract, driver::ProbePurpose::Extract},
    {Prober::ProbePurpose::InjectReextract, driver::ProbePurpose::InjectReextract}};

static const std::map<Prober::State, driver::ProbeState> stateConversion = {
    {Prober::State::Idle, driver::ProbeState::Idle},
    {Prober::State::Owned, driver::ProbeState::Owned},
    {Prober::State::Allocated, driver::ProbeState::Allocated},
    {Prober::State::Active, driver::ProbeState::Active}};

void Prober::throwIfIllegal(const ProbePointId &candidate)
{
    auto type = static_cast<uint32_t>(candidate.type);

    if (candidate.moduleTypeId >= 1 << driver::ProbePointId::moduleIdSize) {
        throw Exception("Module id too large (" + std::to_string(candidate.moduleTypeId) + ").");
    }

    if (candidate.moduleInstanceId >= 1 << driver::ProbePointId::instanceIdSize) {
        throw Exception("Instance id too large (" + std::to_string(candidate.moduleInstanceId) +
                        ").");
    }

    if (type >= 1 << driver::ProbePointId::typeSize) {
        throw Exception("Type too large (" + std::to_string(type) + ").");
    }

    if (candidate.pinIndex >= 1 << driver::ProbePointId::indexSize) {
        throw Exception("Pin index too large (" + std::to_string(candidate.pinIndex) + ").");
    }
}

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

driver::ProbePointId Prober::toWindows(const Prober::ProbePointId &from)
{
    throwIfIllegal(from);

    return {from.moduleTypeId, from.moduleInstanceId, static_cast<uint32_t>(from.type),
            from.pinIndex};
}

Prober::ProbePointId Prober::fromWindows(const driver::ProbePointId &from)
{
    const auto &fields = from.fields;
    return {fields.moduleId, fields.instanceId, static_cast<ProbeType>(fields.type), fields.index};
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

void Prober::setState(State state)
{
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
        driver::ProbePointId probePointId = toWindows(probe.probePoint);
        connections.emplace_back(probe.enabled, probePointId, toWindows(probe.purpose),
                                 eventHandles.injectionHandles[probeIndex].get());

        ++probeIndex;
    }

    driver::ProbePointConfiguration toDriver;
    toDriver.extractionBufferCompletionEventHandle = eventHandles.extractionHandle.get();
    std::copy_n(connections.data(), driver::maxProbes, toDriver.probePointConnection);
    return toDriver;
}

void Prober::setSessionProbes(const SessionProbes probes)
{
    ioctl<SetProbePointConfiguration>(toWindows(probes, mEventHandles));
}

Prober::SessionProbes Prober::getSessionProbes()
{
    driver::ProbePointConfiguration from;
    memset(&from, 0xFF, sizeof(from));
    ioctl<GetProbePointConfiguration>(from);

    SessionProbes result;
    for (const auto &connection : from.probePointConnection) {
        result.emplace_back(connection.enabled, fromWindows(connection.probePointId),
                            fromWindows(connection.purpose));
    }
    return result;
}

std::unique_ptr<util::Buffer> Prober::dequeueExtractionBlock(ProbeId probeIndex)
{
    /* TO DO */
    return nullptr;
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
    util::ByteStreamWriter bodyPayloadWriter;
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
        util::ByteStreamReader reader(bodyPayloadBuffer);
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
}
}
}
