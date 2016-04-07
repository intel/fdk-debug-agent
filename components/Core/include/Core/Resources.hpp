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

#include "Core/TypeModel.hpp"
#include "Core/InstanceModel.hpp"
#include "Core/ParameterDispatcher.hpp"
#include "Rest/Resource.hpp"
#include "cAVS/System.hpp"
#include "Util/Locker.hpp"

namespace debug_agent
{
namespace core
{

using ExclusiveInstanceModel = util::Locker<std::shared_ptr<InstanceModel>>;

class SystemResource : public rest::Resource
{
public:
    explicit SystemResource(cavs::System &system) : mSystem(system) {}
protected:
    cavs::System &mSystem;
};

/** This resource returns the System Type, containing Subsystems Types (XML) */
class SystemTypeResource : public rest::Resource
{
public:
    SystemTypeResource(TypeModel &model) : mTypeModel(model) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    TypeModel &mTypeModel;
};

/** This resource returns the System instance, containing Subsystem instances (XML) */
class SystemInstanceResource : public rest::Resource
{
public:
    SystemInstanceResource(const ifdk_objects::instance::System &systemInstance)
        : mSystemInstance(systemInstance)
    {
    }

protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    const ifdk_objects::instance::System &mSystemInstance;
};

/** This resource returns a subsystem type (XML) */
class TypeResource : public rest::Resource
{
public:
    TypeResource(TypeModel &model) : mTypeModel(model) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    TypeModel &mTypeModel;
};

class InstanceResource : public rest::Resource
{
public:
    InstanceResource(ExclusiveInstanceModel &instanceModel) : mInstanceModel(instanceModel) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    ExclusiveInstanceModel &mInstanceModel;
};

class InstanceCollectionResource : public rest::Resource
{
public:
    InstanceCollectionResource(ExclusiveInstanceModel &instanceModel)
        : mInstanceModel(instanceModel)
    {
    }

protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    ExclusiveInstanceModel &mInstanceModel;
};

class RefreshSubsystemResource : public SystemResource
{
public:
    RefreshSubsystemResource(cavs::System &system, ExclusiveInstanceModel &instanceModel)
        : SystemResource(system), mInstanceModel(instanceModel)
    {
    }

protected:
    virtual ResponsePtr handlePost(const rest::Request &request) override;

private:
    ExclusiveInstanceModel &mInstanceModel;
};

class ParameterStructureResource : public SystemResource
{
public:
    ParameterStructureResource(cavs::System &system, ParameterDispatcher &paramDispatcher,
                               ParameterKind kind)
        : SystemResource(system), mParamDispatcher(paramDispatcher), mKind(kind)
    {
    }

protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    ParameterDispatcher &mParamDispatcher;
    ParameterKind mKind;
};

class ParameterValueResource : public SystemResource
{
public:
    ParameterValueResource(cavs::System &system, ParameterDispatcher &paramDispatcher,
                           ParameterKind kind)
        : SystemResource(system), mParamDispatcher(paramDispatcher), mKind(kind)
    {
    }

protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
    virtual ResponsePtr handlePut(const rest::Request &request) override;

private:
    ParameterDispatcher &mParamDispatcher;
    ParameterKind mKind;
};

/** This resource returns the Log Stream for a service Instance (XML) */
class LogServiceStreamResource : public SystemResource
{
public:
    LogServiceStreamResource(cavs::System &system) : SystemResource(system) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
};

/** This resource extracts and injects probe data */
class ProbeStreamResource : public SystemResource
{
public:
    ProbeStreamResource(cavs::System &system) : SystemResource(system) {}

private:
    static cavs::ProbeId getProbeId(const rest::Request &request);

    ResponsePtr handleGet(const rest::Request &request) override;
    ResponsePtr handlePut(const rest::Request &request) override;
};
}
}
