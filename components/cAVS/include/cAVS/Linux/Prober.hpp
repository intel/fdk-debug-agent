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
