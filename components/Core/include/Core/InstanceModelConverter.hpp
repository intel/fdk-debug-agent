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
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createLogService();
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createPipe();
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createTask();
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createCore();
    std::shared_ptr<ifdk_objects::instance::BaseCollection> createGateway(
        const cavs::dsp_fw::ConnectorNodeId::Type &gatewayType);

    std::shared_ptr<ifdk_objects::instance::BaseCollection> createModule(
        uint32_t moduleId);

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
        std::set<uint32_t> pipeIds;
    };

    /* Parents of modules */
    struct ModuleParents
    {
        std::set<uint32_t> pipeIds;
        std::set<uint32_t> taskIds;
    };

    /* Gateway direction */
    enum class GatewayDirection {
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
    static void addInstanceCollection(InstanceModel::CollectionMap &map,
        const std::string &typeName,
        std::shared_ptr<ifdk_objects::instance::BaseCollection> collection);
};

}
}


