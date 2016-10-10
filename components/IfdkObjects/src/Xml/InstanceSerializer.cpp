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

#include "IfdkObjects/Xml/InstanceSerializer.hpp"
#include <cassert>
#include <sstream>

using namespace debug_agent::ifdk_objects::instance;

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

void InstanceSerializer::enter(const Instance &instance, bool isConcrete)
{
    if (isConcrete) {
        pushElement(instance);
    }
    setAttribute(InstanceTraits<Instance>::attributeTypeName, instance.getTypeName());
    setAttribute(InstanceTraits<Instance>::attributeInstanceId, instance.getInstanceId());
}

void InstanceSerializer::enter(const Component &component, bool isConcrete)
{
    if (isConcrete) {
        pushElement(component);
    }
}

void InstanceSerializer::enter(const Subsystem &subsystem)
{
    pushElement(subsystem);
}

void InstanceSerializer::enter(const System &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const Service &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const EndPoint &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const Ref &ref, bool isConcrete)
{
    assert(!isConcrete);
    setAttribute(InstanceTraits<Ref>::attributeTypeName, ref.getTypeName());
    setAttribute(InstanceTraits<Ref>::attributeInstanceId, ref.getInstanceId());
}

void InstanceSerializer::enter(const InstanceRef &ref)
{
    pushElement(ref);
}

void InstanceSerializer::enter(const ComponentRef &componentRef)
{
    pushElement(componentRef);
}

void InstanceSerializer::enter(const ServiceRef &serviceRef)
{
    pushElement(serviceRef);
}

void InstanceSerializer::enter(const EndPointRef &serviceRef)
{
    pushElement(serviceRef);
}

void InstanceSerializer::enter(const SubsystemRef &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const SystemRef &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const RefCollection &collection, bool isConcrete)
{
    assert(!isConcrete);
    setAttribute(InstanceTraits<RefCollection>::attributeName, collection.getName());
}

void InstanceSerializer::enter(const InstanceRefCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const ComponentRefCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const ServiceRefCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const EndPointRefCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const SubsystemRefCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const Children &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const Parents &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const Parameters &, bool isConcrete)
{
    assert(!isConcrete);
}

void InstanceSerializer::enter(const InfoParameters &parameters)
{
    pushElement(parameters);
}

void InstanceSerializer::enter(const ControlParameters &parameters)
{
    pushElement(parameters);
}

void InstanceSerializer::enter(const Connector &connector, bool isConcrete)
{
    assert(!isConcrete);

    setAttribute(InstanceTraits<Connector>::attributeId, connector.getId());
    setAttribute(InstanceTraits<Connector>::attributeFormat, connector.getFormat());
}

void InstanceSerializer::enter(const Input &connector)
{
    pushElement(connector);
}

void InstanceSerializer::enter(const Output &connector)
{
    pushElement(connector);
}

void InstanceSerializer::enter(const Inputs &connectors)
{
    pushElement(connectors);
}

void InstanceSerializer::enter(const Outputs &connectors)
{
    pushElement(connectors);
}

void InstanceSerializer::enter(const From &instance)
{
    pushElement(instance);
    setAttribute(InstanceTraits<From>::attributeTypeName, instance.getTypeName());
    setAttribute(InstanceTraits<From>::attributeInstanceId, instance.getInstanceId());
    setAttribute(InstanceTraits<From>::attributeOutputId, instance.getOutputId());
}

void InstanceSerializer::enter(const To &instance)
{
    pushElement(instance);
    setAttribute(InstanceTraits<To>::attributeTypeName, instance.getTypeName());
    setAttribute(InstanceTraits<To>::attributeInstanceId, instance.getInstanceId());
    setAttribute(InstanceTraits<To>::attributeInputId, instance.getInputId());
}

void InstanceSerializer::enter(const Link &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const Links &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const InstanceCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const ComponentCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const SubsystemCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const ServiceCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::enter(const EndPointCollection &instance)
{
    pushElement(instance);
}

void InstanceSerializer::leave(bool isConcrete)
{
    if (isConcrete) {
        popElement();
    }
}
}
}
}
