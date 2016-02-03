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

namespace debug_agent
{
namespace cavs
{
namespace windows
{

Prober::State Prober::toCavs(const driver::ProbeState &fromDriver)
{
    switch (fromDriver) {
    case driver::ProbeState::Idle:
        return State::Idle;
    case driver::ProbeState::Owned:
        return State::Owned;
    case driver::ProbeState::Allocated:
        return State::Allocated;
    case driver::ProbeState::Active:
        return State::Active;
    }
    throw Exception("Wrong state value.");
}

driver::ProbeState Prober::fromCavs(const State &toDriver)
{
    switch (toDriver) {
    case State::Idle:
        return driver::ProbeState::Idle;
    case State::Owned:
        return driver::ProbeState::Owned;
    case State::Allocated:
        return driver::ProbeState::Allocated;
    case State::Active:
        return driver::ProbeState::Active;
    }
    throw Exception("Wrong state value.");
}

void Prober::setState(State state)
{
    auto tmp = fromCavs(state);
    ioctl<SetState>(tmp);
}

Prober::State Prober::getState()
{
    // Initialize with an illegal value in case the driver fails to overwrite it.
    driver::ProbeState state{static_cast<driver::ProbeState>(-1)};
    ioctl<GetState>(state);

    return toCavs(state);
}

void Prober::setSessionProbes(const std::vector<ProbeConfig> probes)
{
    /* TO DO */
}

std::vector<Prober::ProbeConfig> Prober::getSessionProbes()
{
    /* TO DO */
    return std::vector<ProbeConfig>();
}

std::unique_ptr<util::Buffer> Prober::dequeueExtractionBlock(uint32_t probeIndex)
{
    /* TO DO */
    return nullptr;
}

bool Prober::enqueueInjectionBlock(uint32_t probeIndex, const util::Buffer &buffer)
{
    /* TO DO */
    return false;
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
    util::Buffer buffer = IoctlHelpers::toTinyCmdBuffer(
        static_cast<uint32_t>(driver::IOCTL_FEATURE::FEATURE_PROBE_CAPTURE), T::id,
        bodyPayloadWriter.getBuffer());

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
