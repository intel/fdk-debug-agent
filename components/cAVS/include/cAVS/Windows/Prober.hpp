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
#include "cAVS/DspFw/Probe.hpp"
#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/IoCtlDescription.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/BlockingQueue.hpp"
#include "cAVS/Windows/EventHandle.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/Probe/Extractor.hpp"

#include <array>
#include <memory>
#include <set>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

class Prober : public cavs::Prober
{
public:
    /** Contains all probe event handles */
    struct EventHandles
    {
        EventHandles() = default;
        EventHandles(EventHandles &&) = default;
        EventHandles(const EventHandles &) = delete;
        EventHandles &operator=(const EventHandles &) = delete;

        bool isValid() const
        {
            if (extractionHandle->handle() == nullptr) {
                return false;
            }
            for (auto &handle : injectionHandles) {
                if (handle->handle() == nullptr) {
                    return false;
                }
            }
            return true;
        }

        using HandlePtr = std::unique_ptr<EventHandle>;
        using HandlePtrArray = std::array<HandlePtr, driver::maxProbes>;

        HandlePtr extractionHandle;
        HandlePtrArray injectionHandles;
    };

    /** This factory creates event handles using the type provided by the template parameter */
    template <typename HandleT>
    struct EventHandlesFactory
    {

        static EventHandles createHandles()
        {
            EventHandles handles;
            handles.extractionHandle = makeHandle();
            for (auto &ptr : handles.injectionHandles) {
                ptr = makeHandle();
            }
            return handles;
        }

        static EventHandles::HandlePtr makeHandle() { return std::make_unique<HandleT>(); }
    };

    using SystemEventHandlesFactory = EventHandlesFactory<SystemEventHandle>;

    /** Create a driver probe config from os-agnostic config and event handles */
    static driver::ProbePointConfiguration toWindows(
        const cavs::Prober::SessionProbes &probes,
        const windows::Prober::EventHandles &eventHandles);

    Prober(Device &device, const EventHandles &eventHandles);

    std::size_t getMaxProbeCount() const override { return driver::maxProbes; }
    void setState(State state) override;
    State getState() override;
    void setSessionProbes(const SessionProbes probes,
                          std::map<ProbeId, std::size_t> injectionSampleByteSizes) override;
    SessionProbes getSessionProbes() override;
    std::unique_ptr<util::Buffer> dequeueExtractionBlock(ProbeId probeIndex) override;
    bool enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer) override;

    driver::RingBuffersDescription getRingBuffers();
    size_t getExtractionRingBufferLinearPosition();

private:
    static constexpr auto mProbeFeature = driver::IOCTL_FEATURE::FEATURE_FW_PROBE;
    static constexpr std::size_t mQueueSize = 5 * 1024 * 1024;

    using PacketQueue = util::BlockingQueue<util::Buffer>;

    // 0 = get/setState
    using GetState =
        IoCtlDescription<driver::IoCtlType::TinyGet, mProbeFeature,
                         driver::ProbeFeatureParameter::FEATURE_STATE, driver::ProbeState>;
    using SetState =
        IoCtlDescription<driver::IoCtlType::TinySet, mProbeFeature,
                         driver::ProbeFeatureParameter::FEATURE_STATE, driver::ProbeState>;
    // 1 = get/setProbePointConfiguration
    using GetProbePointConfiguration =
        IoCtlDescription<driver::IoCtlType::TinyGet, mProbeFeature,
                         driver::ProbeFeatureParameter::POINT_CONFIGURATION,
                         driver::ProbePointConfiguration>;
    using SetProbePointConfiguration =
        IoCtlDescription<driver::IoCtlType::TinySet, mProbeFeature,
                         driver::ProbeFeatureParameter::POINT_CONFIGURATION,
                         driver::ProbePointConfiguration>;
    using GetRingBuffersDescription =
        IoCtlDescription<driver::IoCtlType::TinyGet, mProbeFeature,
                         driver::ProbeFeatureParameter::BUFFERS_DESCRIPTION,
                         driver::RingBuffersDescription>;
    using GetExtractionRingBufferPosition =
        IoCtlDescription<driver::IoCtlType::TinyGet, mProbeFeature,
                         driver::ProbeFeatureParameter::EXTRACTION_BUFFER_STATUS, uint64_t>;
    /** Send a probes-related ioctl to the driver
     *
     * @tparam T A type describing the ioctl (id, direction, type of the data
     *           to be sent - described by a Data member).
     * @param[in,out] inout Reference to the data to be sent/received.
     */
    template <class T>
    void ioctl(typename T::Data &inout);

    /** Same as previous ioctl() method, but with dynamic params */
    template <class T>
    void ioctl(driver::IoCtlType type, ULONG feature, ULONG parameterId, T &inout);

    /** Convert values from OS-agnostic cAVS to cAVS Windows driver and vice-versa
     */
    /** @{ */
    static driver::ProbeState toWindows(const State &from);
    static State fromWindows(const driver::ProbeState &from);
    static driver::ProbePurpose toWindows(const ProbePurpose &from);
    static ProbePurpose fromWindows(const driver::ProbePurpose &from);
    static BOOL toWindows(bool value);
    static bool fromWindows(BOOL value);
    /** @} */

    void checkProbeId(ProbeId id) const;

    void startStreaming();
    void stopStreaming();

    /** @return active probe indexes (extraction/injection) */
    std::pair<std::set<ProbeId> /*Extract*/, std::set<ProbeId> /*Inject*/> getActiveProbes() const;

    Device &mDevice;
    const EventHandles &mEventHandles;
    SessionProbes mCachedProbeConfiguration;
    std::map<ProbeId, std::size_t> mCachedInjectionSampleByteSizes;
    std::vector<PacketQueue> mExtractionQueues;
    std::unique_ptr<probe::Extractor> mExtractor;
};
}
}
}
