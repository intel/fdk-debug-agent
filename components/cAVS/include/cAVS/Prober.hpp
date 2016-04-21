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

#include "DspFw/Probe.hpp"

#include "Util/Exception.hpp"
#include "Util/WrappedRaw.hpp"

#include <memory>
#include <vector>
#include <set>

namespace debug_agent
{
namespace cavs
{

namespace detail
{
struct ProbeIdTrait
{
    using RawType = uint32_t;
};
}

using ProbeId = util::WrappedRaw<detail::ProbeIdTrait>;

/**
 * This interface intends to abstract probing OS-specificities
 *
 * An important choice has been made: this interface lets the os-specific implementation decide
 * and implement its own threading model (the threading model that uses the driver and
 * fills/consumes audio block queues).
 *
 * This choice has been made because Linux driver api (based on tiny-alsa) and windows api (based
 * on ioctl) are very heterogeneous.
 *
 * The drawback is that the os-specific logic is larger and more complex
 *
 * @todo: write usage sequence diagram
 *
 */
class Prober
{
public:
    using Exception = util::Exception<Prober>;

    enum class ProbePurpose
    {
        Inject,
        Extract,
        InjectReextract
    };

    static const util::EnumHelper<ProbePurpose> &probePurposeHelper()
    {
        static const util::EnumHelper<ProbePurpose> helper({
            {ProbePurpose::Inject, "Inject"},
            {ProbePurpose::Extract, "Extract"},
            {ProbePurpose::InjectReextract, "InjectReextract"},
        });
        return helper;
    }

    /** Configuration of one probe instance */
    struct ProbeConfig
    {
        bool enabled;
        dsp_fw::ProbePointId probePoint; /* The probe point id where the probe will be connected */
        ProbePurpose purpose;            /* The probe purpose (inject, extract, both) */

        ProbeConfig() = default;
        ProbeConfig(bool enabled, const dsp_fw::ProbePointId &probePoint, ProbePurpose purpose)
            : enabled(enabled), probePoint(probePoint), purpose(purpose)
        {
        }

        bool operator==(const ProbeConfig &other) const
        {
            return enabled == other.enabled && probePoint == other.probePoint &&
                   purpose == other.purpose;
        }
    };

    /** Maps endpoint IDs to probe configurations */
    using SessionProbes = std::vector<ProbeConfig>;

    /** @return active probe indexes (extraction/injection) */
    static std::pair<std::set<ProbeId> /*Extract*/, std::set<ProbeId> /*Inject*/> getActiveSession(
        const SessionProbes &probeSessions)
    {
        std::set<ProbeId> extractionProbes;
        std::set<ProbeId> injectionProbes;

        ProbeId::RawType probeIndex{0};
        for (auto &probe : probeSessions) {
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

    Prober() = default;

    virtual ~Prober() = default;

    /** Set service state
     * @throw Prober::Exception
     */
    virtual void setState(bool active) = 0;

    /** Get service state
     * @throw Prober::Exception
     */
    virtual bool isActive() = 0;

    /** Set configuration of one probe
     * @param[in] injectionSampleByteSizes sample byte size used to inject silence if underrun
     * happens.
     *
     * @throw Prober::Exception if the probe id is wrong
     */
    virtual void setProbeConfig(ProbeId id, const ProbeConfig &config,
                                std::size_t injectionSampleByteSize) = 0;

    /** Get configuration of one probe
     * @throw Prober::Exception if the probe id is wrong
     */
    virtual ProbeConfig getProbeConfig(ProbeId id) const = 0;

    /** @return max supported probe count */
    virtual std::size_t getMaxProbeCount() const = 0;

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
    virtual std::unique_ptr<util::Buffer> dequeueExtractionBlock(ProbeId probeIndex) = 0;

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
    virtual bool enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer) = 0;

    Prober(const Prober &) = delete;
    Prober &operator=(const Prober &) = delete;
};
}
}
