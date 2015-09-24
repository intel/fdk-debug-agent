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
    ModuleListDebugResource(cavs::System &system) :
        SystemResource(system) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
};

/** This debug resource dumps cAVS topology */
class TopologyDebugResource : public SystemResource
{
public:
    TopologyDebugResource(cavs::System &system) :
        SystemResource(system) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;

private:
    /* These methods dump topology elements */
    void dumpGateways(HtmlHelper &html, const std::vector<cavs::dsp_fw::GatewayProps> &gateways);
    void dumpPipelines(HtmlHelper &html, const std::vector<cavs::dsp_fw::PplProps> &pipelines);
    void dumpAllSchedulers(HtmlHelper &html,
        const std::vector<cavs::dsp_fw::SchedulersInfo> &allSchedulers);
    void dumpCoreSchedulers(HtmlHelper &html,
        const cavs::dsp_fw::SchedulersInfo &coreSchedulers);
    void dumpTasks(HtmlHelper &html, const std::vector<cavs::dsp_fw::TaskProps> &tasks);
    void dumpModuleInstances(HtmlHelper &html,
        const std::map<cavs::dsp_fw::CompoundModuleId, cavs::dsp_fw::ModuleInstanceProps>
        &moduleInstances);
    void dumpPins(HtmlHelper &html, const std::vector<cavs::dsp_fw::PinProps> &pins);
};

/** This debug resource dumps model cache to the fdk tool mock format */
class ModelDumpDebugResource : public rest::Resource
{
public:
    ModelDumpDebugResource(const TypeModel &typeModel,
        const ifdk_objects::instance::System &systemInstance,
        ExclusiveInstanceModel &instanceModel) :
        mTypeModel(typeModel),
        mSystemInstance(systemInstance),
        mInstanceModel(instanceModel) {}
protected:
    virtual ResponsePtr handleGet(const rest::Request &request) override;
private:
    const TypeModel &mTypeModel;
    const ifdk_objects::instance::System &mSystemInstance;
    ExclusiveInstanceModel &mInstanceModel;
};

}
}