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

const std::string InstanceTraits<EndPointRef>::tag("endpoint");

const std::string InstanceTraits<SubsystemRef>::tag("subsystem");

const std::string InstanceTraits<SystemRef>::tag("system");

const std::string InstanceTraits<Instance>::tag("instance");
const std::string InstanceTraits<Instance>::attributeTypeName("Type");
const std::string InstanceTraits<Instance>::attributeInstanceId("Id");

const std::string InstanceTraits<Component>::tag("component");

const std::string InstanceTraits<Subsystem>::tag("subsystem");

const std::string InstanceTraits<System>::tag("system");

const std::string InstanceTraits<Service>::tag("service");
const std::string InstanceTraits<Service>::attributeDirection("Direction");

const std::string InstanceTraits<EndPoint>::tag("endpoint");

const std::string InstanceTraits<Parents>::tag("parents");

const std::string InstanceTraits<Children>::tag("children");

const std::string InstanceTraits<RefCollection>::attributeName("Name");

const std::string InstanceTraits<InstanceRefCollection>::tag("collection");

const std::string InstanceTraits<ComponentRefCollection>::tag("component_collection");

const std::string InstanceTraits<ServiceRefCollection>::tag("service_collection");

const std::string InstanceTraits<EndPointRefCollection>::tag("endpoint_collection");

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
const std::string InstanceTraits<ServiceCollection>::tag("service_collection");
const std::string InstanceTraits<EndPointCollection>::tag("endpoint_collection");
}
}
}
