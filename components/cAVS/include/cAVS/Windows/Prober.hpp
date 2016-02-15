/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
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

#include "cAVS/Prober.hpp"
#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/IoCtlDescription.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "cAVS/Windows/EventHandle.hpp"
#include "cAVS/Windows/DriverTypes.hpp"

#include <array>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

class Prober : public cavs::Prober
{
public:
    using EventArray = std::array<EventHandle, driver::maxProbes>;

    struct EventHandles
    {
        EventHandle extractionHandle;
        EventArray injectionHandles;
    };

    /** Create a driver probe config from os-agnostic config and event handles */
    static driver::ProbePointConfiguration toWindows(
        const cavs::Prober::SessionProbes &probes,
        const windows::Prober::EventHandles &eventHandles);

    Prober(Device &device, const EventHandles &eventHandles)
        : mDevice(device), mEventHandles(eventHandles)
    {
    }

    std::size_t getMaxProbeCount() const override { return driver::maxProbes; }
    void setState(State state) override;
    State getState() override;
    void setSessionProbes(const SessionProbes probes) override;
    SessionProbes getSessionProbes() override;
    std::unique_ptr<util::Buffer> dequeueExtractionBlock(ProbeId probeIndex) override;
    bool enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer) override;

    driver::RingBuffersDescription getRingBuffers();

private:
    static constexpr auto mProbeFeature = driver::IOCTL_FEATURE::FEATURE_PROBE_CAPTURE;

    // 0 = get/setState
    using GetState =
        IoCtlDescription<driver::IoCtlType::TinyGet, mProbeFeature, 0, driver::ProbeState>;
    using SetState =
        IoCtlDescription<driver::IoCtlType::TinySet, mProbeFeature, 0, driver::ProbeState>;
    // 1 = get/setProbePointConfiguration
    using GetProbePointConfiguration = IoCtlDescription<driver::IoCtlType::TinyGet, mProbeFeature,
                                                        1, driver::ProbePointConfiguration>;
    using SetProbePointConfiguration = IoCtlDescription<driver::IoCtlType::TinySet, mProbeFeature,
                                                        1, driver::ProbePointConfiguration>;
    using GetRingBuffersDescription = IoCtlDescription<driver::IoCtlType::TinyGet, mProbeFeature, 2,
                                                       driver::RingBuffersDescription>;

    /** Send a probes-related ioctl to the driver
     *
     * @tparam T A type describing the ioctl (id, direction, type of the data
     *           to be sent - described by a Data member).
     * @param[in,out] inout Reference to the data to be sent/received.
     */
    template <class T>
    void ioctl(typename T::Data &inout);

    static void throwIfIllegal(const ProbePointId &candidate);

    /** Convert values from OS-agnostic cAVS to cAVS Windows driver and vice-versa
     */
    /** @{ */
    static driver::ProbeState toWindows(const State &from);
    static State fromWindows(const driver::ProbeState &from);
    static driver::ProbePointId toWindows(const ProbePointId &from);
    static ProbePointId fromWindows(const driver::ProbePointId &from);
    static driver::ProbePurpose toWindows(const ProbePurpose &from);
    static ProbePurpose fromWindows(const driver::ProbePurpose &from);
    /** @} */

    Device &mDevice;
    const EventHandles &mEventHandles;
};
}
}
}
