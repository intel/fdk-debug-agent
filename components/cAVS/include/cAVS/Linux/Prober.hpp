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
    Prober(ControlDevice &controlDevice, CompressDeviceFactory &compressDeviceFactory);
    ~Prober() override;

    std::size_t getMaxProbeCount() const override { return mixer_ctl::maxProbes; }

    void setState(bool active) override;

    bool isActive() override;

    void setProbeConfig(ProbeId id, const ProbeConfig &config,
                        std::size_t injectionSampleByteSize) override;

    ProbeConfig getProbeConfig(ProbeId id) const override;

    std::unique_ptr<util::Buffer> dequeueExtractionBlock(ProbeId probeIndex) override;

    bool enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer) override;

    static ProbeConfig fromLinux(const mixer_ctl::ProbeControl &from);
    static mixer_ctl::ProbeControl toLinux(const ProbeConfig &from);

private:
    void startStreaming();
    void stopStreaming();

    void checkProbeId(ProbeId id) const;

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
    SessionProbes mCachedProbeConfiguration;
    std::map<ProbeId, std::size_t> mCachedInjectionSampleByteSizes;

    /** Extraction of multiplexed probe points is performed by a compress device. */
    std::unique_ptr<ProbeExtractor> mProbeExtractor;
    std::vector<ProbeInjector> mProbeInjectors;

    BlockingExtractionQueues mExtractionQueues;
    std::vector<util::RingBuffer> mInjectionQueues;

    bool mIsActiveState;

    mutable std::mutex mServiceStateMutex;
    mutable std::mutex mProbeConfigMutex;
};
}
}
}
