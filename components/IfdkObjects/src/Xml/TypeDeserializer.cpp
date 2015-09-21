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

void TypeDeserializer::enter(Categories &categories)
{
    pushElement(categories);
    fillPolymorphicVector<Ref, TypeRef, ComponentRef, ServiceRef, SubsystemRef>
        (categories.getElements());
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

void TypeDeserializer::enter(SubsystemRefCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void TypeDeserializer::enter(Children &chidren)
{
    pushElement(chidren);
    fillPolymorphicVector<RefCollection, TypeRefCollection, ComponentRefCollection,
        ServiceRefCollection, SubsystemRefCollection>(chidren.getElements());
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

void TypeDeserializer::enter(Parameters &parameters, bool isConcrete)
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


