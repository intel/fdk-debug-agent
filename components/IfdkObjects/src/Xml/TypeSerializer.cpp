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

#include "IfdkObjects/Xml/TypeSerializer.hpp"

using namespace debug_agent::ifdk_objects::type;

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

void TypeSerializer::enter(const Type &type, bool isConcrete)
{
    if (isConcrete) {
        pushElement(type);
    }
    setAttribute(TypeTraits<Type>::attributeName, type.getName());
}

void TypeSerializer::enter(const Component &component, bool isConcrete)
{
    if (isConcrete) {
        pushElement(component);
    }
}

void TypeSerializer::enter(const Subsystem &subsystem)
{
    pushElement(subsystem);
}

void TypeSerializer::enter(const System &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const Service &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const EndPoint &instance)
{
    pushElement(instance);

    std::string directionName = EndPoint::directionHelper().toString(instance.getDirection());
    setAttribute(TypeTraits<EndPoint>::attributeDirection, directionName);
}

void TypeSerializer::enter(const Categories &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const Ref &ref, bool isConcrete)
{
    assert(!isConcrete);
    setAttribute(TypeTraits<Ref>::attributeName, ref.getRefName());
}

void TypeSerializer::enter(const TypeRef &ref)
{
    pushElement(ref);
}

void TypeSerializer::enter(const ComponentRef &componentRef)
{
    pushElement(componentRef);
}

void TypeSerializer::enter(const ServiceRef &serviceRef)
{
    pushElement(serviceRef);
}

void TypeSerializer::enter(const EndPointRef &serviceRef)
{
    pushElement(serviceRef);
}

void TypeSerializer::enter(const SubsystemRef &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const RefCollection &collection, bool isConcrete)
{
    assert(!isConcrete);
    setAttribute(TypeTraits<RefCollection>::attributeName, collection.getName());
}

void TypeSerializer::enter(const TypeRefCollection &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const ComponentRefCollection &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const ServiceRefCollection &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const EndPointRefCollection &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const SubsystemRefCollection &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const Children &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const Characteristic &characteristic)
{
    pushElement(characteristic);
    setAttribute(TypeTraits<Characteristic>::attributeName, characteristic.getName());
    setText(characteristic.getValue());
}

void TypeSerializer::enter(const Characteristics &instance)
{
    pushElement(instance);
}

void TypeSerializer::enter(const Description &description)
{
    pushElement(description);
    setText(description.getValue());
}

void TypeSerializer::enter(const Parameters &, bool isConcrete)
{
    assert(!isConcrete);
}

void TypeSerializer::enter(const InfoParameters &parameters)
{
    pushElement(parameters);
}

void TypeSerializer::enter(const ControlParameters &parameters)
{
    pushElement(parameters);
}

void TypeSerializer::enter(const Connector &connector, bool isConcrete)
{
    assert(!isConcrete);

    setAttribute(TypeTraits<Connector>::attributeId, connector.getId());
    setAttribute(TypeTraits<Connector>::attributeName, connector.getName());
}

void TypeSerializer::enter(const Input &connector)
{
    pushElement(connector);
}

void TypeSerializer::enter(const Output &connector)
{
    pushElement(connector);
}

void TypeSerializer::enter(const Inputs &connectors)
{
    pushElement(connectors);
}

void TypeSerializer::enter(const Outputs &connectors)
{
    pushElement(connectors);
}

void TypeSerializer::leave(bool isConcrete)
{
    if (isConcrete) {
        popElement();
    }
}
}
}
}
