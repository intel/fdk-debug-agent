/*
 * Copyright (c) 2015, Intel Corporation
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

#include "Core/Resources.hpp"
#include "cAVS/Topology.hpp"

namespace debug_agent
{
namespace core
{

/* Forward declaration of an internal type */
class HtmlHelper;

/** This debug resource lists module types */
class ModuleListDebugResource : public SystemResource
{
public:
    ModuleListDebugResource(cavs::System &system) : SystemResource(system) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
};

/** This debug resource dumps cAVS topology */
class TopologyDebugResource : public SystemResource
{
public:
    TopologyDebugResource(cavs::System &system) : SystemResource(system) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    /* These methods dump topology elements */
    void dumpGateways(HtmlHelper &html, const std::vector<cavs::dsp_fw::GatewayProps> &gateways);
    void dumpPipelines(HtmlHelper &html, const std::vector<cavs::dsp_fw::PplProps> &pipelines);
    void dumpAllSchedulers(HtmlHelper &html,
                           const std::vector<cavs::dsp_fw::SchedulersInfo> &allSchedulers);
    void dumpCoreSchedulers(HtmlHelper &html, const cavs::dsp_fw::SchedulersInfo &coreSchedulers);
    void dumpTasks(HtmlHelper &html, const std::vector<cavs::dsp_fw::TaskProps> &tasks);
    void dumpModuleInstances(HtmlHelper &html,
                             const std::map<cavs::dsp_fw::CompoundModuleId,
                                            cavs::dsp_fw::ModuleInstanceProps> &moduleInstances);
    void dumpPins(HtmlHelper &html, const std::vector<cavs::dsp_fw::PinProps> &pins);
};

/** This debug resource dumps model cache to the fdk tool mock format */
class ModelDumpDebugResource : public rest::Resource
{
public:
    ModelDumpDebugResource(const TypeModel &typeModel,
                           const ifdk_objects::instance::System &systemInstance,
                           ExclusiveInstanceModel &instanceModel)
        : mTypeModel(typeModel), mSystemInstance(systemInstance), mInstanceModel(instanceModel)
    {
    }

protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    const TypeModel &mTypeModel;
    const ifdk_objects::instance::System &mSystemInstance;
    ExclusiveInstanceModel &mInstanceModel;
};

/** This resource returns general information about a Debug Agent's instance */
class AboutResource : public rest::Resource
{
private:
    ResponsePtr handleGet(const rest::Request &request) override;
};
}
}
