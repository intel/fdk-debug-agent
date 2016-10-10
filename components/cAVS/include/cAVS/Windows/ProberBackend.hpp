/*
 * Copyright (c) 2015, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include "cAVS/DspFw/Probe.hpp"
#include "cAVS/Prober.hpp"
#include "cAVS/ProbeExtractor.hpp"
#include "cAVS/ProbeInjector.hpp"
#include "cAVS/Windows/Device.hpp"
#include "cAVS/Windows/IoCtlDescription.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/BlockingQueue.hpp"
#include "Util/Exception.hpp"
#include "Util/RingBuffer.hpp"
#include "cAVS/Windows/EventHandle.hpp"
#include "cAVS/Windows/DriverTypes.hpp"

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
     * Prober Backend state shall be 'Owned'.
     *
     * @param[in] probes the probe configuration
     * param[in] injectionSampleByteSizes A <probe index,  sample byte size> map used to inject
     *                                     silence if underrun happens
     *
     * @throw Prober::Exception
     */
    void setSessionProbes(const cavs::Prober::SessionProbes &probes,
                          const cavs::Prober::InjectionSampleByteSizes &injectionSampleByteSizes);

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
    cavs::Prober::InjectionSampleByteSizes mCachedInjectionSampleByteSizes;
    ProbeExtractor::BlockingExtractionQueues mExtractionQueues;
    std::unique_ptr<ProbeExtractor> mExtractor;

    std::vector<util::RingBuffer> mInjectionQueues;
    std::vector<ProbeInjector> mInjectors;

    mutable std::mutex mProbeConfigMutex;
};
}
}
}
