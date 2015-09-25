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

#include "Core/Resources.hpp"
#include "ParameterMgrPlatformConnector.h"
#include "ElementHandle.h"
#include <memory>

namespace debug_agent
{
namespace core
{
/** Base class for resources using PFW */
class PFWResource : public SystemResource
{
public:
    PFWResource(cavs::System &system,
        CParameterMgrPlatformConnector &parameterMgrPlatformConnector);
protected:
    const CParameterMgrPlatformConnector &getParameterMgrPlatformConnector() const
    {
        return mParameterMgrPlatformConnector;
    }
private:
    CParameterMgrPlatformConnector &mParameterMgrPlatformConnector;

};

/** Base class for module resources*/
class ModuleResource : public PFWResource
{
public:
    ModuleResource(cavs::System &system,
        CParameterMgrPlatformConnector &parameterMgrPlatformConnector,
        const std::string moduleName,
        const uint16_t moduleId) :
        PFWResource(system, parameterMgrPlatformConnector),
        mModuleName(moduleName),
        mModuleId(moduleId) {}

protected:
    std::unique_ptr<CElementHandle> getModuleControlElement() const;
    std::unique_ptr<CElementHandle> getChildElementHandle(
        const CElementHandle &moduleElementHandle, uint32_t childId) const;
    uint32_t getElementMapping(const CElementHandle &elementHandle) const;
    const std::string mModuleName;
    const uint16_t mModuleId;
};

/** This resource returns control parameters of a module instance of a Subsystem (XML) */
class ControlParametersModuleInstanceResource : public ModuleResource
{
public:
    ControlParametersModuleInstanceResource(cavs::System &system,
        CParameterMgrPlatformConnector &parameterMgrPlatformConnector,
        const std::string moduleName,
        const uint16_t moduleId) :
        ModuleResource(system, parameterMgrPlatformConnector, moduleName, moduleId) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
    virtual ResponsePtr handlePut(const rest::Request &request) override;
};

/** This resource returns control parameters of a module type of a Subsystem (XML) */
class ControlParametersModuleTypeResource : public ModuleResource
{
public:
    ControlParametersModuleTypeResource(cavs::System &system,
        CParameterMgrPlatformConnector &parameterMgrPlatformConnector,
        const std::string moduleName,
        const uint16_t moduleId) :
        ModuleResource(system, parameterMgrPlatformConnector, moduleName, moduleId) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
};

}
}