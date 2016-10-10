/*
 * Copyright (c) 2015-2016, Intel Corporation
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

#include "Core/BaseModelConverter.hpp"
#include "Core/InstanceModel.hpp"
#include "cAVS/System.hpp"
#include <set>
#include <map>

namespace debug_agent
{
namespace core
{

/** This class converts cAVS data model to generic instance data model */
class InstanceModelConverter final : public BaseModelConverter
{
public:
    InstanceModelConverter(cavs::System &system) : BaseModelConverter(system) {}

    static std::shared_ptr<ifdk_objects::instance::System> createSystem();
    std::shared_ptr<InstanceModel> createModel();

private:
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createSubsystem();
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createPipe();
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createTask();
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createCore();
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createGateway(
        const cavs::dsp_fw::ConnectorNodeId::Type &gatewayType);

    std::shared_ptr<ifdk_objects::instance::BaseCollection> createModule(uint16_t moduleId);

    std::shared_ptr<ifdk_objects::instance::RefCollection> createModuleRef(
        const std::vector<cavs::dsp_fw::CompoundModuleId> &compoundIdList);

    /* This method calculates intermediate structures required to compute instance model:
     * - parents of modules and tasks
     * - peers connected to a gateway
     */
    void initializeIntermediateStructures();

    /* Parents of tasks */
    struct TaskParents
    {
        std::set<cavs::dsp_fw::PipeLineIdType> pipeIds;
    };

    /* Parents of modules */
    struct ModuleParents
    {
        std::set<cavs::dsp_fw::PipeLineIdType> pipeIds;
        std::set<uint32_t> taskIds;
    };

    /* Gateway direction */
    enum class GatewayDirection
    {
        Input,
        Output
    };

    using TaskParentMap = std::map<uint32_t, TaskParents>;
    using ModuleParentMap = std::map<cavs::dsp_fw::CompoundModuleId, ModuleParents>;
    using GatewayDirectionMap = std::map<uint32_t, GatewayDirection>;

    cavs::Topology mTopology;

    /* Intermediate structures */
    TaskParentMap mTaskParents;
    ModuleParentMap mModuleParents;
    GatewayDirectionMap mGatewayDirections;

    /** Add one instance collection in the supplied map. The key of the map is :
     *  <subsystem name>.<type name>
     */
    static void addInstanceCollection(
        InstanceModel::CollectionMap &map, const std::string &typeName,
        std::shared_ptr<ifdk_objects::instance::BaseCollection> collection);

    /** Add all instances required by a service */
    static void addServiceInstanceCollection(InstanceModel::CollectionMap &map,
                                             const std::string &serviceTypeName,
                                             std::size_t endPointCount);

    /** Common method to create service instance */
    static std::shared_ptr<ifdk_objects::instance::BaseCollection> createService(
        const std::string &serviceTypeName, std::size_t endPointCount);

    /** Common method to create endpoint instance */
    static std::shared_ptr<ifdk_objects::instance::BaseCollection> createEndPoint(
        const std::string &serviceTypeName, std::size_t endPointCount);
};
}
}
