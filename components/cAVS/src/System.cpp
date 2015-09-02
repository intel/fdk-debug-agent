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
#include "cAVS/System.hpp"
#include "cAVS/DriverFactory.hpp"
#include "cAVS/LogStreamer.hpp"
#include <utility>
#include <set>

namespace debug_agent
{
namespace cavs
{

System::System(const DriverFactory &driverFactory):
    mDriver(std::move(createDriver(driverFactory))),
    mModuleEntries(),
    mFwConfig(),
    mHwConfig()
{
    if (mDriver == nullptr) {

        throw Exception("Driver factory has failed");
    }
    try
    {
        mDriver->getModuleHandler().getFwConfig(mFwConfig);
    }
    catch (ModuleHandler::Exception &e)
    {
        /** @todo use logging */
        std::cout << "Unable to get FW config: " + std::string(e.what()) << std::endl;
    }
    try
    {
        mDriver->getModuleHandler().getHwConfig(mHwConfig);
    }
    catch (ModuleHandler::Exception &e)
    {
        /** @todo use logging */
        std::cout << "Unable to get HW config: " + std::string(e.what()) << std::endl;
    }

    if (mFwConfig.isModulesCountValid) {
        try
        {
            mDriver->getModuleHandler().getModulesEntries(mFwConfig.modulesCount, mModuleEntries);
        }
        catch (ModuleHandler::Exception &e)
        {
            /** @todo use logging */
            std::cout << "Unable to get module entries: " + std::string(e.what()) << std::endl;
        }
    }
    else {
        /** @todo use logging */
        std::cout << "Cannot get module entries: module count is invalid.";
    }
}

std::unique_ptr<Driver> System::createDriver(const DriverFactory &driverFactory)
{
    try
    {
        return driverFactory.newDriver();
    }
    catch (DriverFactory::Exception e)
    {
        throw Exception("Unable to create driver: " + std::string(e.what()));
    }
}

void System::setLogParameters(Logger::Parameters &parameters)
{
    try
    {
        mDriver->getLogger().setParameters(parameters);
    }
    catch (Logger::Exception &e)
    {
        throw Exception("Unable to set log parameter: " + std::string(e.what()));
    }
}

Logger::Parameters System::getLogParameters()
{
    try
    {
        return mDriver->getLogger().getParameters();
    }
    catch (Logger::Exception &e)
    {
        throw Exception("Unable to get log parameter: " + std::string(e.what()));
    }
}

const std::vector<ModuleEntry> &System::getModuleEntries() const NOEXCEPT
{
    return mModuleEntries;
}

const FwConfig &System::getFwConfig() const NOEXCEPT
{
    return mFwConfig;
}

const HwConfig &System::getHwConfig() const NOEXCEPT
{
    return mHwConfig;
}

std::unique_ptr<System::LogStreamResource> System::tryToAcquireLogStreamResource()
{
    std::unique_ptr<System::LogStreamResource> resource(new System::LogStreamResource(*this));
    if (resource->tryLock()) {
        return resource;
    }
    else {
        return nullptr;
    }
}

void System::doLogStreamInternal(std::ostream &os)
{
    LogStreamer logStreamer(mDriver->getLogger(), mModuleEntries);

    os << logStreamer;
}

void System::setModuleParameter(uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
    const std::vector<uint8_t> &parameterPayload)
{
    mDriver->getModuleHandler().setModuleParameter(moduleId, instanceId, parameterId,
        parameterPayload);
}

void System::getModuleParameter(uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
    std::vector<uint8_t> &parameterPayload)
{
    mDriver->getModuleHandler().getModuleParameter(moduleId, instanceId, parameterId,
        parameterPayload);
}

void System::getTopology(Topology &topology)
{
    topology.clear();

    ModuleHandler &handler = mDriver->getModuleHandler();
    std::set<uint32_t> moduleInstanceIds;

    /* Retrieving gateways*/
    if (!mHwConfig.isGatewayCountValid) {
        throw Exception("Gate count is invalid.");
    }

    try
    {
        handler.getGatewaysInfo(mHwConfig.gatewayCount, topology.gateways);
    }
    catch (ModuleHandler::Exception &e)
    {
        throw Exception("Can not retrieve gateways: " + std::string(e.what()));
    }

    /* Retrieving pipelines ids*/
    if (!mFwConfig.isMaxPplCountValid) {
        throw Exception("Max pipeline count is invalid.");
    }

    std::vector<uint32_t> pipelineIds;
    try
    {
        handler.getPipelineIdList(mFwConfig.maxPplCount, pipelineIds);
    }
    catch (ModuleHandler::Exception &e)
    {
        throw Exception("Can not retrieve pipeline ids: " + std::string(e.what()));
    }

    /* Retrieving pipeline props*/
    for (auto pplId : pipelineIds) {
        try {
            DSPplProps props;
            handler.getPipelineProps(pplId, props);
            topology.pipelines.push_back(props);

            /* Collecting module instance ids*/
            for (auto instanceId : props.module_instances) {
                moduleInstanceIds.insert(instanceId);
            }
        }
        catch (ModuleHandler::Exception &e)
        {
            throw Exception("Can not retrieve pipeline props of id " + std::to_string(pplId) +
                " : " + std::string(e.what()));
        }
    }

    /* Retrieving scheduler props*/
    if (!mHwConfig.isDspCoreCountValid) {
        throw Exception("Core count is invalid.");
    }

    for (uint32_t coreId = 0; coreId < mHwConfig.dspCoreCount; coreId++) {
        try
        {
            DSSchedulersInfo info;
            handler.getSchedulersInfo(coreId, info);
            topology.schedulers.push_back(info);

            /* Collecting module instance ids*/
            for (auto &scheduler : info.scheduler_info) {
                for (auto &task : scheduler.task_info) {
                    for (auto instanceId : task.module_instance_id) {
                        moduleInstanceIds.insert(instanceId);
                    }
                }
            }
        }
        catch (ModuleHandler::Exception &e)
        {
            throw Exception("Can not retrieve scheduler props of core id: " +
                std::to_string(coreId) + " : " + std::string(e.what()));
        }
    }

    /* Retrieving module instances*/
    for (auto moduleInstanceId : moduleInstanceIds) {
        try
        {
            uint16_t moduleId, instanceId;
            Topology::splitModuleInstanceId(moduleInstanceId, moduleId, instanceId);

            DSModuleInstanceProps props;
            handler.getModuleInstanceProps(moduleId, instanceId, props);
            topology.moduleInstances[props.id] = props;
        }
        catch (ModuleHandler::Exception &e)
        {
            throw Exception("Can not retrieve module instance with id: " +
                std::to_string(moduleInstanceId) + " : " + std::string(e.what()));
        }
    }

    /* Compute links */
    topology.computeLinks();
}

}
}


