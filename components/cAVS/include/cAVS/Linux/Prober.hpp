/*
 * Copyright (c) 2016, Intel Corporation
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

#include "cAVS/Linux/ControlDevice.hpp"
#include "cAVS/Linux/ControlDeviceTypes.hpp"
#include "cAVS/Linux/CompressTypes.hpp"
#include "cAVS/Linux/CompressDeviceFactory.hpp"

#include "cAVS/ProbeExtractor.hpp"
#include "cAVS/ProbeInjector.hpp"
#include "cAVS/Prober.hpp"

#include "Util/BlockingQueue.hpp"

#include <mutex>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

class Prober final : public cavs::Prober
{
public:
    /** Need to map id of the DBGA on id used for injection / extraction for compress device Id
     * and control mixer associated. We do use the same ProbeId strong type.
     */
    using ProbeControlMap = std::map<ProbeId /*Prober*/, ProbeId /*Probe Control Id*/>;

    Prober(ControlDevice &controlDevice, CompressDeviceFactory &compressDeviceFactory);
    ~Prober() override;

    /**
     * Max probe point supported is the sum of injection and extraction end points.
     * If a probe point is used in injection / extraction, it shall be considered as 2 active
     * probe points from FW point of view.
     * @return max probe allowed.
     */
    std::size_t getMaxProbeCount() const override
    {
        return mMaxInjectionProbes + mMaxExtractionProbes;
    }

    void setState(bool active) override;

    bool isActive() override;

    void setProbesConfig(const SessionProbes &probes,
                         const InjectionSampleByteSizes &injectionSampleByteSizes) override;

    /** Get probes for the current/future session.
     * It retrieves information from the Driver/FW and not from cached configuration.
     *
     * @throw Prober::Exception
     */
    SessionProbes getProbesConfig() const;

    std::unique_ptr<util::Buffer> dequeueExtractionBlock(ProbeId probeIndex) override;

    bool enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer) override;

    static ProbeConfig fromLinux(const mixer_ctl::ProbeControl &from);
    static mixer_ctl::ProbeControl toLinux(const ProbeConfig &from);

private:
    ProbeId getInjectProbeControlId(const ProbeId &probeIndex) const;
    ProbeId getExtractProbeControlId(const ProbeId &probeIndex) const;

    void startStreaming();
    void stopStreaming();

    void stopNoThrow() noexcept;

    using BlockingPacketQueue = util::BlockingQueue<util::Buffer>;
    using BlockingExtractionQueues = std::vector<BlockingPacketQueue>;

    /** All probe streams will work with 5 meg Queues, aligned with windows adaptation layer. */
    static const std::size_t mQueueSize = 5 * 1024 * 1024;

    static mixer_ctl::ProbeState toLinux(bool from);
    static bool fromLinux(const mixer_ctl::ProbeState &from);
    static mixer_ctl::ProbePurpose toLinux(const ProbePurpose &from);
    static ProbePurpose fromLinux(const mixer_ctl::ProbePurpose &from);

    /** Probe configuration is done through control mixer so using control device. */
    ControlDevice &mControlDevice;

    /** Extraction / Injection of stream shall be done through specific compres device, so using
     * the compress device factory to create the compress device handle on the right device.
     */
    CompressDeviceFactory &mCompressDeviceFactory;

    /** Collection of probe configurations with their state, ID, purpose*/
    SessionProbes mCachedProbeConfig;

    ProbeControlMap mExtractionProbeMap;
    ProbeControlMap mInjectionProbeMap;
    InjectionSampleByteSizes mCachedInjectionSampleByteSizes;

    /** Extraction of multiplexed probe points is performed by a compress device. */
    std::unique_ptr<ProbeExtractor> mProbeExtractor;
    std::vector<ProbeInjector> mProbeInjectors;

    size_t mMaxInjectionProbes;
    size_t mMaxExtractionProbes;

    BlockingExtractionQueues mExtractionQueues;
    std::vector<util::RingBuffer> mInjectionQueues;

    bool mIsActiveState;

    mutable std::mutex mServiceStateMutex;
    mutable std::mutex mProbeConfigMutex;
};
}
}
}
