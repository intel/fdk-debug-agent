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
    try
    {
        mDriver->getModuleHandler().getModulesEntries(mModuleEntries);
    }
    catch (ModuleHandler::Exception &e)
    {
        /** @todo use logging */
        std::cout << "Unable to get module entries: " + std::string(e.what()) << std::endl;
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

}
}


