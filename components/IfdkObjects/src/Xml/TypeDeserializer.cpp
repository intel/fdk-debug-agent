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

#include "IfdkObjects/Xml/TypeDeserializer.hpp"

using namespace debug_agent::ifdk_objects::type;

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

void TypeDeserializer::enter(Type &type, bool isConcrete)
{
    if (isConcrete) {
        pushElement(type);
    }
    type.setName(getStringAttribute(TypeTraits<Type>::attributeName));
}

void TypeDeserializer::enter(Component &component, bool isConcrete)
{
    if (isConcrete) {
        pushElement(component);
    }
}

void TypeDeserializer::enter(Subsystem &subsystem)
{
    pushElement(subsystem);
}

void TypeDeserializer::enter(System &instance)
{
    pushElement(instance);
}

void TypeDeserializer::enter(Service &instance)
{
    pushElement(instance);
}

void TypeDeserializer::enter(EndPoint &instance)
{
    pushElement(instance);

    std::string directionName = getStringAttribute(TypeTraits<EndPoint>::attributeDirection);

    EndPoint::Direction direction;
    if (!EndPoint::directionHelper().fromString(directionName, direction)) {
        throw Exception("Invalid service direction: " + directionName);
    }

    instance.setDirection(direction);
}

void TypeDeserializer::enter(Categories &categories)
{
    pushElement(categories);
    fillPolymorphicVector<Ref, TypeRef, ComponentRef, ServiceRef, EndPointRef, SubsystemRef>(
        categories.getElements());
}

void TypeDeserializer::enter(Ref &ref, bool isConcrete)
{
    assert(!isConcrete);
    ref.setRefName(getStringAttribute(TypeTraits<Ref>::attributeName));
}

void TypeDeserializer::enter(TypeRef &ref)
{
    pushElement(ref);
}

void TypeDeserializer::enter(ComponentRef &component)
{
    pushElement(component);
}

void TypeDeserializer::enter(ServiceRef &service)
{
    pushElement(service);
}

void TypeDeserializer::enter(EndPointRef &service)
{
    pushElement(service);
}

void TypeDeserializer::enter(SubsystemRef &instance)
{
    pushElement(instance);
}

void TypeDeserializer::enter(RefCollection &collection, bool isConcrete)
{
    assert(!isConcrete);
    collection.setName(getStringAttribute(TypeTraits<RefCollection>::attributeName));
}

template <class T>
void TypeDeserializer::collectionCommon(GenericRefCollection<T> &collection)
{
    collection.resize(getChildElementCount());
}

void TypeDeserializer::enter(TypeRefCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void TypeDeserializer::enter(ComponentRefCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void TypeDeserializer::enter(ServiceRefCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void TypeDeserializer::enter(EndPointRefCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void TypeDeserializer::enter(SubsystemRefCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void TypeDeserializer::enter(Children &chidren)
{
    pushElement(chidren);
    fillPolymorphicVector<RefCollection, TypeRefCollection, ComponentRefCollection,
                          ServiceRefCollection, EndPointRefCollection, SubsystemRefCollection>(
        chidren.getElements());
}

void TypeDeserializer::enter(Characteristic &characteristic)
{
    pushElement(characteristic);

    characteristic.setName(getStringAttribute(TypeTraits<Characteristic>::attributeName));
    characteristic.setValue(getText());
}

void TypeDeserializer::enter(Characteristics &characteristics)
{
    pushElement(characteristics);
    characteristics.resize(getChildElementCount());
}

void TypeDeserializer::enter(Description &desc)
{
    pushElement(desc);

    desc.setValue(getText());
}

void TypeDeserializer::enter(Parameters &, bool isConcrete)
{
    assert(!isConcrete);
}

void TypeDeserializer::enter(InfoParameters &parameters)
{
    pushElement(parameters);
}

void TypeDeserializer::enter(ControlParameters &parameters)
{
    pushElement(parameters);
}

void TypeDeserializer::enter(Connector &connector, bool isConcrete)
{
    assert(!isConcrete);
    connector.setId(getStringAttribute(TypeTraits<Connector>::attributeId));
    connector.setName(getStringAttribute(TypeTraits<Connector>::attributeName));
}

void TypeDeserializer::enter(Input &connector)
{
    pushElement(connector);
}

void TypeDeserializer::enter(Output &connector)
{
    pushElement(connector);
}

void TypeDeserializer::enter(Inputs &connectors)
{
    pushElement(connectors);
    connectors.resize(getChildElementCount());
}

void TypeDeserializer::enter(Outputs &connectors)
{
    pushElement(connectors);
    connectors.resize(getChildElementCount());
}

void TypeDeserializer::leave(bool isConcrete)
{
    if (isConcrete) {
        popElement();
    }
}
}
}
}
