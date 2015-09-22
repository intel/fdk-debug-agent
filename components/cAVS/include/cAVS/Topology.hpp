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

#include "cAVS/DynamicSizedFirmwareTypes.hpp"
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
    class Exception final : public std::logic_error
    {
    public:
        explicit Exception(const std::string &what)
        : std::logic_error(what)
        {}
    };

    Topology() = default;
    Topology(const Topology&) = default;
    Topology& operator=(const Topology&) = default;

    /* Describe a link between two modules */
    struct Link
    {
        Link(const dsp_fw::CompoundModuleId &fromModuleInstanceId, uint32_t fromOutputId,
            const dsp_fw::CompoundModuleId &toModuleInstanceId, uint32_t toInputId) :
            mFromModuleInstanceId(fromModuleInstanceId), mFromOutputId(fromOutputId),
            mToModuleInstanceId(toModuleInstanceId), mToInputId(toInputId)
        {}
        Link(const Link&) = default;
        Link& operator=(const Link&) = default;

        bool operator==(const Link &other) const
        {
            return
                mFromModuleInstanceId == other.mFromModuleInstanceId &&
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
        return moduleInstances == other.moduleInstances &&
            pipelines == other.pipelines &&
            schedulers == other.schedulers &&
            links == other.links;
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

    std::map<dsp_fw::CompoundModuleId, dsp_fw::DSModuleInstanceProps> moduleInstances;
    std::vector<dsp_fw::GatewayProps> gateways;
    std::vector<dsp_fw::DSPplProps> pipelines;
    std::vector<dsp_fw::DSSchedulersInfo> schedulers;
    std::vector<Link> links;

private:
    /**
     * @param moduleInstanceId the CompoundModuleId of the wanted module instance
     * @return a const reference to the DSModuleInstanceProps which correspond to the
     *         moduleInstanceId.
     * @see CompoundModuleId
     * @throw Topology::Exception
     */
    const dsp_fw::DSModuleInstanceProps &getModuleInstance(
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
        InputList &unresolvedInputs,
        OutputList &unresolvedOutputs);

    void computeInterPipeLinks(InputList &unresolvedInputs, OutputList &unresolvedOutputs);
    void checkUnresolved(InputList &unresolvedInputs, OutputList &unresolvedOutputs) const;
    void addAllModuleOutputs(OutputList &list, const dsp_fw::CompoundModuleId &module) const;
    void addAllModuleInputs(InputList &list, const dsp_fw::CompoundModuleId &module) const;
};

}
}
