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
#include <vector>
#include <map>
#include <assert.h>

namespace debug_agent
{
namespace cavs
{

/* Describe cAVS topology */
struct Topology
{
    using ModuleCompoundId = uint32_t;

    Topology() = default;
    Topology(const Topology&) = default;
    Topology& operator=(const Topology&) = default;

    /* Describe a link between two modules */
    struct Link
    {
        Link(uint32_t pFromModuleInstanceId, uint32_t pFromOutputId, uint32_t pToModuleInstanceId,
            uint32_t pToInputId) :
            fromModuleInstanceId(pFromModuleInstanceId), fromOutputId(pFromOutputId),
            toModuleInstanceId(pToModuleInstanceId), toInputId(pToInputId) {}
        Link(const Link&) = default;
        Link& operator=(const Link&) = default;

        bool operator==(const Link &other) const
        {
            return fromModuleInstanceId == other.fromModuleInstanceId &&
                fromOutputId == other.fromOutputId &&
                toModuleInstanceId == other.toModuleInstanceId &&
                toInputId == other.toInputId;
        }

        uint32_t fromModuleInstanceId;
        uint32_t fromOutputId;

        uint32_t toModuleInstanceId;
        uint32_t toInputId;
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
            links == other.links &&
            gatewayPeers == other.gatewayPeers;
    }

    void clear()
    {
        moduleInstances.clear();
        gateways.clear();
        pipelines.clear();
        schedulers.clear();
        links.clear();
        gatewayPeers.clear();
    }

    static uint32_t joinModuleInstanceId(uint16_t moduleId, uint16_t instanceId)
    {
        return (static_cast<uint32_t>(moduleId) << 16) | instanceId;
    }

    static void splitModuleInstanceId(uint32_t moduleInstanceId,
        uint16_t &moduleId, uint16_t &instanceId)
    {
        instanceId = moduleInstanceId & 0xFFFF;
        moduleId = moduleInstanceId >> 16;
    }

    std::map<ModuleCompoundId, DSModuleInstanceProps> moduleInstances;
    std::vector<dsp_fw::GatewayProps> gateways;
    std::vector<DSPplProps> pipelines;
    std::vector<DSSchedulersInfo> schedulers;
    std::vector<Link> links;

    /* a gateway peer is a module instance id connected to a gateway */
    std::vector<uint32_t> gatewayPeers;
};

/* Calculate links and gateway peers of the supplied topology*/
class LinkCalculator
{
public:
    LinkCalculator(Topology &topology) : mTopology(topology) {}

    void computeLinksAndGatewayPeers();

private:
    Topology &mTopology;
};

}
}
