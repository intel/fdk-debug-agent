/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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
#include "ParameterSerializer/ParameterSerializer.hpp"
#include "Util/Locker.hpp"
#include "cAVS/DspFw/Common.hpp"
#include <memory>

namespace debug_agent
{
namespace core
{
/** Base class for module resources*/
class ModuleResource : public SystemResource
{
public:
    ModuleResource(cavs::System &system,
                   util::Locker<parameter_serializer::ParameterSerializer> &parameterSerializer,
                   const std::string moduleName, const uint16_t moduleId)
        : SystemResource(system), mParameterSerializer(parameterSerializer),
          mModuleName(moduleName), mModuleId(moduleId)
    {
    }

protected:
    std::map<uint32_t, std::string> getChildren(
        parameter_serializer::ParameterSerializer::ParameterKind parameterKind) const;
    uint16_t getInstanceId(const rest::Request &request) const;
    cavs::dsp_fw::ParameterId getParamId(
        parameter_serializer::ParameterSerializer::ParameterKind parameterKind,
        const std::string parameterName) const;

    util::Locker<parameter_serializer::ParameterSerializer> &mParameterSerializer;
    const std::string mModuleName;
    const uint16_t mModuleId;
};

/** This resource returns control parameters of a module instance of a Subsystem (XML) */
class ControlParametersModuleInstanceResource : public ModuleResource
{
public:
    ControlParametersModuleInstanceResource(
        cavs::System &system,
        util::Locker<parameter_serializer::ParameterSerializer> &parameterSerializer,
        const std::string moduleName, const uint16_t moduleId)
        : ModuleResource(system, parameterSerializer, moduleName, moduleId)
    {
    }

protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
    virtual ResponsePtr handlePut(const rest::Request &request) override;
};

/** This resource returns control parameters of a module type of a Subsystem (XML) */
class ControlParametersModuleTypeResource : public ModuleResource
{
public:
    ControlParametersModuleTypeResource(
        cavs::System &system,
        util::Locker<parameter_serializer::ParameterSerializer> &parameterSerializer,
        const std::string moduleName, const uint16_t moduleId)
        : ModuleResource(system, parameterSerializer, moduleName, moduleId)
    {
    }

protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
};
}
}
