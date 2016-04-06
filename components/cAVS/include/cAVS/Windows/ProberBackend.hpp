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

#include "cAVS/DspFw/Probe.hpp"
#include "cAVS/Prober.hpp"
#include "cAVS/ProbeExtractor.hpp"
#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/IoCtlDescription.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/BlockingQueue.hpp"
#include "Util/Exception.hpp"
#include "Util/RingBuffer.hpp"
#include "cAVS/Windows/EventHandle.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/Probe/Injector.hpp"

#include <array>
#include <memory>
#include <set>
#include <vector>
#include <mutex>
#include <map>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

class ProberBackend
{
public:
    using Exception = util::Exception<ProberBackend>;

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
    static driver::ProbePointConfiguration toWindows(const cavs::Prober::SessionProbes &probes,
                                                     const EventHandles &eventHandles);

    /** State of the probing service */
    enum class State
    {
        Idle,
        Owned,     /// probe service is owned (required because it's a monoclient service)
        Allocated, /// Buffers are allocated
        Active     /// Probing is running
    };

    static const util::EnumHelper<State> &stateHelper()
    {
        static const util::EnumHelper<State> helper({
            {State::Idle, "Idle"},
            {State::Owned, "Owned"},
            {State::Allocated, "Allocated"},
            {State::Active, "Active"},
        });
        return helper;
    }

    ProberBackend(Device &device, const EventHandles &eventHandles);
    ProberBackend() = default;

    /** @return max supported probe count */
    std::size_t getMaxProbeCount() const { return driver::maxProbes; }

    /** Set the state transition of the probing backend.
     *
     * The state machine specified in the SwAS must be respected.
     *
     * @todo draw state machine
     * @throw ProberBackend::Exception
     */
    void setStateTransition(State newState, State oldState);

    /**
     * Get the state of the probing service
     *
     * @throw ProberBackend::Exception
     */
    State getState();

    /** Set probes for the future session.
     *
     * Probe service state shall be 'Owned'.
     *
     * @throw ProberBackend::Exception
     */
    void setProbeConfig(ProbeId id, const cavs::Prober::ProbeConfig &config,
                        std::size_t injectionSampleByteSize);

    cavs::Prober::ProbeConfig getProbeConfig(ProbeId id) const;

    /** Set probes for the future session.
     *
     * Prober Backend state shall be 'Owned'.
     *
     * @param[in] probes the probe configuration
     * param[in] injectionSampleByteSizes A <probe index,  sample byte size> map used to inject
     *                                     silence if underrun happens
     *
     * @throw Prober::Exception
     */
    void setSessionProbes(const cavs::Prober::SessionProbes &probes,
                          std::map<ProbeId, std::size_t> injectionSampleByteSizes);

    /** Get probes for the current/future session.
     *
     * Prober Backend state shall be in 'Owned, Allocated, Active'.
     *
     * @throw ProberBackend::Exception
     */
    cavs::Prober::SessionProbes getSessionProbes();

    /**
     * Return the next extraction block data
     *
     * This method blocks until data is available (or probe is stopped)
     *
     * Probe service state shall be in 'Active'.
     *
     * @param[in] probeIndex the index of the probe to query
     * @return the next extraction block data, or nullptr if the probe has been stopped.
     *
     * @throw Prober::Exception
     */
    std::unique_ptr<util::Buffer> dequeueExtractionBlock(ProbeId probeIndex);

    /**
     * Enqueue a block that will be injected to the probe.
     *
     * Probe service state shall be in 'Active'.
     *
     * @param[in] probeIndex the index of the probe to query
     * @param[out] buffer the block to inject
     * @return true if the block has been enqueued, of false if the probe is closed.
     *
     * @throw Prober::Exception
     */
    bool enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer);

private:
    ProberBackend(const ProberBackend &) = delete;
    ProberBackend &operator=(const ProberBackend &) = delete;

    void setSessionProbes();

    /** Convert values from OS-agnostic cAVS to cAVS Windows driver and vice-versa
     */
    /** @{ */
    static driver::ProbePurpose toWindows(const cavs::Prober::ProbePurpose &from);
    static cavs::Prober::ProbePurpose fromWindows(const driver::ProbePurpose &from);
    static BOOL toWindows(bool value);
    static bool fromWindows(BOOL value);
    /** @} */

    /* Check if probe id is in valid range */
    void checkProbeId(ProbeId id) const;

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

    /** Convert values from OS-agnostic cAVS to cAVS Windows driver and vice-versa
     */
    /** @{ */
    static driver::ProbeState toWindows(const State &from);
    static State fromWindows(const driver::ProbeState &from);
    /** @} */

    void startStreaming();
    void stopStreaming();

    driver::RingBuffersDescription getRingBuffers();
    size_t getExtractionRingBufferLinearPosition();
    size_t getInjectionRingBufferLinearPosition(ProbeId probeId);

    Device &mDevice;
    const ProberBackend::EventHandles &mEventHandles;
    cavs::Prober::SessionProbes mCachedProbeConfiguration;

    /** map that provides the sample byte size of each injection probes. */
    std::map<ProbeId, std::size_t> mCachedInjectionSampleByteSizes;
    ProbeExtractor::BlockingExtractionQueues mExtractionQueues;
    std::unique_ptr<ProbeExtractor> mExtractor;

    std::vector<util::RingBuffer> mInjectionQueues;
    std::vector<probe::Injector> mInjectors;

    mutable std::mutex mProbeConfigMutex;
};
}
}
}
