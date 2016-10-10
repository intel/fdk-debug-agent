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

    using InjectionSampleByteSizes = std::map<ProbeId, std::size_t>;

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

    /** Set configuration of probes
     * @param[in] probes configuration of all active probe sessions.
     * @param[in] injectionSampleByteSizes sample byte size used to inject silence if underrun
     * happens.
     *
     * @throw Prober::Exception if the probe id is wrong
     */
    virtual void setProbesConfig(const SessionProbes &probes,
                                 const InjectionSampleByteSizes &injectionSampleByteSizes) = 0;

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
