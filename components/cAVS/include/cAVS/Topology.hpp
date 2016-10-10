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

#include "cAVS/DspFw/Gateway.hpp"
#include "cAVS/DspFw/Pipeline.hpp"
#include "cAVS/DspFw/ModuleInstance.hpp"
#include "cAVS/DspFw/ModuleType.hpp"
#include "cAVS/DspFw/Scheduler.hpp"
#include <utility>
#include <vector>
#include <map>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{

/* Describe cAVS topology */
class Topology
{
public:
    struct Exception final : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    Topology() = default;
    Topology(const Topology &) = default;
    Topology &operator=(const Topology &) = default;

    /* Describe a link between two modules */
    struct Link
    {
        Link(const dsp_fw::CompoundModuleId &fromModuleInstanceId, uint32_t fromOutputId,
             const dsp_fw::CompoundModuleId &toModuleInstanceId, uint32_t toInputId)
            : mFromModuleInstanceId(fromModuleInstanceId), mFromOutputId(fromOutputId),
              mToModuleInstanceId(toModuleInstanceId), mToInputId(toInputId)
        {
        }
        Link(const Link &) = default;
        Link &operator=(const Link &) = default;

        bool operator==(const Link &other) const
        {
            return mFromModuleInstanceId == other.mFromModuleInstanceId &&
                   mFromOutputId == other.mFromOutputId &&
                   mToModuleInstanceId == other.mToModuleInstanceId &&
                   mToInputId == other.mToInputId;
        }

        dsp_fw::CompoundModuleId mFromModuleInstanceId;
        uint32_t mFromOutputId;

        dsp_fw::CompoundModuleId mToModuleInstanceId;
        uint32_t mToInputId;
    };

    bool operator==(const Topology &other) const
    {
        /* The GatewayProps type has no operator == so we can't use equal operator
         * of member gateways */
        if (gateways.size() != other.gateways.size()) {
            return false;
        }

        for (std::size_t i = 0; i < gateways.size(); ++i) {
            if ((gateways[i].id != other.gateways[i].id) ||
                (gateways[i].attribs != other.gateways[i].attribs)) {
                return false;
            }
        }

        /* Using operator == for other members */
        return moduleInstances == other.moduleInstances && pipelines == other.pipelines &&
               schedulers == other.schedulers && links == other.links;
    }

    /** Erase topology's collections of pipes, modules, schedulers, links and gateways */
    void clear()
    {
        moduleInstances.clear();
        gateways.clear();
        pipelines.clear();
        schedulers.clear();
        links.clear();
    }

    /**
     * Compute Links collection from pipes and modules collections
     * @throw Topology::Exception
     */
    void computeLinks();

    std::map<dsp_fw::CompoundModuleId, dsp_fw::ModuleInstanceProps> moduleInstances;
    std::vector<dsp_fw::GatewayProps> gateways;
    std::vector<dsp_fw::PplProps> pipelines;
    std::vector<dsp_fw::SchedulersInfo> schedulers;
    std::vector<Link> links;

private:
    /**
     * @param moduleInstanceId the CompoundModuleId of the wanted module instance
     * @return a const reference to the ModuleInstanceProps which correspond to the
     *         moduleInstanceId.
     * @see CompoundModuleId
     * @throw Topology::Exception
     */
    const dsp_fw::ModuleInstanceProps &getModuleInstance(
        const dsp_fw::CompoundModuleId &moduleInstanceId) const;

    using InputId = uint32_t;
    using Input = std::pair<dsp_fw::CompoundModuleId, InputId>;
    using InputList = std::vector<Input>;

    using OutputId = uint32_t;
    using Output = std::pair<dsp_fw::CompoundModuleId, OutputId>;
    using OutputList = std::vector<Output>;

    void computeIntraPipeLinks(InputList &unresolvedInputs, OutputList &unresolvedOutputs);
    void computeModulesPairLink(const dsp_fw::CompoundModuleId &sourceModuleId,
                                const dsp_fw::CompoundModuleId &destinationModuleId,
                                InputList &unresolvedInputs, OutputList &unresolvedOutputs);

    void computeInterPipeLinks(InputList &unresolvedInputs, OutputList &unresolvedOutputs);
    void checkUnresolved(InputList &unresolvedInputs, OutputList &unresolvedOutputs) const;
    void addAllModuleOutputs(OutputList &list, const dsp_fw::CompoundModuleId &module) const;
    void addAllModuleInputs(InputList &list, const dsp_fw::CompoundModuleId &module) const;
};
}
}
