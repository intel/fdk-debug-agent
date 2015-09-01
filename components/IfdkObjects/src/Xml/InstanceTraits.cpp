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

#include "IfdkObjects/Xml/InstanceTraits.hpp"

using namespace debug_agent::ifdk_objects::instance;

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

const std::string InstanceTraits<Ref>::attributeTypeName("Type");
const std::string InstanceTraits<Ref>::attributeInstanceId("Id");

const std::string InstanceTraits<InstanceRef>::tag("instance");

const std::string InstanceTraits<ComponentRef>::tag("component");

const std::string InstanceTraits<ServiceRef>::tag("service");

const std::string InstanceTraits<SubsystemRef>::tag("subsystem");

const std::string InstanceTraits<SystemRef>::tag("system");

const std::string InstanceTraits<Instance>::tag("instance");
const std::string InstanceTraits<Instance>::attributeTypeName("Type");
const std::string InstanceTraits<Instance>::attributeInstanceId("Id");

const std::string InstanceTraits<Component>::tag("component");

const std::string InstanceTraits<Subsystem>::tag("subsystem");

const std::string InstanceTraits<System>::tag("system");

const std::string InstanceTraits<Parents>::tag("parents");

const std::string InstanceTraits<Children>::tag("children");

const std::string InstanceTraits<RefCollection>::attributeName("Name");

const std::string InstanceTraits<InstanceRefCollection>::tag("collection");

const std::string InstanceTraits<ComponentRefCollection>::tag("component_collection");

const std::string InstanceTraits<ServiceRefCollection>::tag("service_collection");

const std::string InstanceTraits<SubsystemRefCollection>::tag("subsystem_collection");

const std::string InstanceTraits<InfoParameters>::tag("info_parameters");

const std::string InstanceTraits<ControlParameters>::tag("control_parameters");

const std::string InstanceTraits<Connector>::attributeId("Id");
const std::string InstanceTraits<Connector>::attributeFormat("Format");

const std::string InstanceTraits<Input>::tag("input");

const std::string InstanceTraits<Output>::tag("output");

const std::string InstanceTraits<Inputs>::tag("inputs");

const std::string InstanceTraits<Outputs>::tag("outputs");

const std::string InstanceTraits<From>::tag("from");
const std::string InstanceTraits<From>::attributeTypeName("Type");
const std::string InstanceTraits<From>::attributeInstanceId("Id");
const std::string InstanceTraits<From>::attributeOutputId("OutputId");

const std::string InstanceTraits<To>::tag("to");
const std::string InstanceTraits<To>::attributeTypeName("Type");
const std::string InstanceTraits<To>::attributeInstanceId("Id");
const std::string InstanceTraits<To>::attributeInputId("InputId");

const std::string InstanceTraits<Link>::tag("link");

const std::string InstanceTraits<Links>::tag("links");

const std::string InstanceTraits<InstanceCollection>::tag("collection");
const std::string InstanceTraits<ComponentCollection>::tag("component_collection");
const std::string InstanceTraits<SubsystemCollection>::tag("subsystem_collection");
}
}
}


