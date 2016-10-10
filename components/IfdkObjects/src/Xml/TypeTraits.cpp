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

#include "IfdkObjects/Xml/TypeTraits.hpp"

using namespace debug_agent::ifdk_objects::type;

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

const std::string TypeTraits<Ref>::attributeName("Name");

const std::string TypeTraits<TypeRef>::tag("type");

const std::string TypeTraits<ComponentRef>::tag("component_type");

const std::string TypeTraits<ServiceRef>::tag("service_type");

const std::string TypeTraits<EndPointRef>::tag("endpoint_type");

const std::string TypeTraits<SubsystemRef>::tag("subsystem_type");

const std::string TypeTraits<Type>::tag("type");
const std::string TypeTraits<Type>::attributeName("Name");

const std::string TypeTraits<Component>::tag("component_type");

const std::string TypeTraits<Subsystem>::tag("subsystem_type");

const std::string TypeTraits<System>::tag("system_type");

const std::string TypeTraits<Service>::tag("service_type");

const std::string TypeTraits<EndPoint>::tag("endpoint_type");
const std::string TypeTraits<EndPoint>::attributeDirection("Direction");

const std::string TypeTraits<Categories>::tag("categories");

const std::string TypeTraits<Children>::tag("children");

const std::string TypeTraits<RefCollection>::attributeName("Name");

const std::string TypeTraits<TypeRefCollection>::tag("collection");

const std::string TypeTraits<ComponentRefCollection>::tag("component_collection");

const std::string TypeTraits<ServiceRefCollection>::tag("service_collection");

const std::string TypeTraits<EndPointRefCollection>::tag("endpoint_collection");

const std::string TypeTraits<SubsystemRefCollection>::tag("subsystem_collection");

const std::string TypeTraits<Characteristic>::tag("characteristic");
const std::string TypeTraits<Characteristic>::attributeName("Name");

const std::string TypeTraits<Characteristics>::tag("characteristics");

const std::string TypeTraits<Description>::tag("description");

const std::string TypeTraits<InfoParameters>::tag("info_parameters");

const std::string TypeTraits<ControlParameters>::tag("control_parameters");

const std::string TypeTraits<Connector>::attributeId("Id");
const std::string TypeTraits<Connector>::attributeName("Name");

const std::string TypeTraits<Input>::tag("input");

const std::string TypeTraits<Output>::tag("output");

const std::string TypeTraits<Inputs>::tag("inputs");

const std::string TypeTraits<Outputs>::tag("outputs");
}
}
}
