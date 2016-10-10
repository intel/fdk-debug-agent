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
