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

#include "IfdkObjects/Xml/InstanceDeserializer.hpp"

using namespace Poco;
using namespace Poco::XML;
using namespace debug_agent::ifdk_objects::instance;

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

void InstanceDeserializer::enter(Instance &type, bool isConcrete)
{
    if (isConcrete) {
        pushElement(type);
    }
    type.setTypeName(getStringAttribute(InstanceTraits<Instance>::attributeTypeName));
    type.setInstanceId(getStringAttribute(InstanceTraits<Instance>::attributeInstanceId));
}

void InstanceDeserializer::enter(Component &component, bool isConcrete)
{
    if (isConcrete) {
        pushElement(component);
    }
}

void InstanceDeserializer::enter(Subsystem &subsystem)
{
    pushElement(subsystem);
}

void InstanceDeserializer::enter(System &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(Service &instance)
{
    pushElement(instance);

    std::string directionName = getStringAttribute(InstanceTraits<Service>::attributeDirection);

    Service::Direction direction;
    if (!Service::directionHelper().fromString(directionName, direction)) {
        throw Exception("Invalid service direction: " + directionName);
    }

    instance.setDirection(direction);
}

void InstanceDeserializer::enter(Ref &ref, bool isConcrete)
{
    assert(!isConcrete);
    ref.setTypeName(getStringAttribute(InstanceTraits<Ref>::attributeTypeName));
    ref.setInstanceId(getStringAttribute(InstanceTraits<Ref>::attributeInstanceId));
}

void InstanceDeserializer::enter(InstanceRef &ref)
{
    pushElement(ref);
}

void InstanceDeserializer::enter(ComponentRef &component)
{
    pushElement(component);
}

void InstanceDeserializer::enter(ServiceRef &service)
{
    pushElement(service);
}

void InstanceDeserializer::enter(SubsystemRef &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(SystemRef &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(RefCollection &collection, bool isConcrete)
{
    assert(!isConcrete);
    collection.setName(getStringAttribute(InstanceTraits<RefCollection>::attributeName));
}

template <class T>
void InstanceDeserializer::refCollectionCommon(GenericRefCollection<T> &collection)
{
    collection.resize(getChildElementCount());
}

void InstanceDeserializer::enter(InstanceRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(ComponentRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(ServiceRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(SubsystemRefCollection &instance)
{
    pushElement(instance);
    refCollectionCommon(instance);
}

void InstanceDeserializer::enter(Children &chidren)
{
    pushElement(chidren);
    fillPolymorphicVector<RefCollection, InstanceRefCollection, ComponentRefCollection,
        ServiceRefCollection, SubsystemRefCollection>(chidren.getElements());
}

void InstanceDeserializer::enter(Parents &parents)
{
    pushElement(parents);
    fillPolymorphicVector<Ref, InstanceRef, ComponentRef, SubsystemRef,
        ServiceRef>(parents.getElements());
}

void InstanceDeserializer::enter(Parameters &parameters, bool isConcrete)
{
    assert(!isConcrete);
}

void InstanceDeserializer::enter(InfoParameters &parameters)
{
    pushElement(parameters);
}

void InstanceDeserializer::enter(ControlParameters &parameters)
{
    pushElement(parameters);
}

void InstanceDeserializer::enter(Connector &connector, bool isConcrete)
{
    assert(!isConcrete);
    connector.setId(getStringAttribute(InstanceTraits<Connector>::attributeId));
    connector.setFormat(getStringAttribute(InstanceTraits<Connector>::attributeFormat));
}

void InstanceDeserializer::enter(Input &connector)
{
    pushElement(connector);
}

void InstanceDeserializer::enter(Output &connector)
{
    pushElement(connector);
}

void InstanceDeserializer::enter(Inputs &connectors)
{
    pushElement(connectors);
    connectors.resize(getChildElementCount());
}

void InstanceDeserializer::enter(Outputs &connectors)
{
    pushElement(connectors);
    connectors.resize(getChildElementCount());
}

void InstanceDeserializer::enter(From &instance)
{
    pushElement(instance);
    instance.setTypeName(getStringAttribute(InstanceTraits<From>::attributeTypeName));
    instance.setInstanceId(getStringAttribute(InstanceTraits<From>::attributeInstanceId));
    instance.setOutputId(getStringAttribute(InstanceTraits<From>::attributeOutputId));
}

void InstanceDeserializer::enter(To &instance)
{
    pushElement(instance);
    instance.setTypeName(getStringAttribute(InstanceTraits<To>::attributeTypeName));
    instance.setInstanceId(getStringAttribute(InstanceTraits<To>::attributeInstanceId));
    instance.setInputId(getStringAttribute(InstanceTraits<To>::attributeInputId));
}

void InstanceDeserializer::enter(Link &instance)
{
    pushElement(instance);
}

void InstanceDeserializer::enter(Links &instance)
{
    pushElement(instance);
    instance.resize(getChildElementCount());
}

template <class T>
void InstanceDeserializer::collectionCommon(GenericCollection<T> &collection)
{
    std::size_t childCount = getChildElementCount();
    for (std::size_t i = 0; i < childCount; ++i) {
        collection.add(std::make_shared<T>());
    }
}

void InstanceDeserializer::enter(InstanceCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::enter(ComponentCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::enter(SubsystemCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::enter(ServiceCollection &instance)
{
    pushElement(instance);
    collectionCommon(instance);
}

void InstanceDeserializer::leave(bool isConcrete)
{
    if (isConcrete) {
        popElement();
    }
}

}
}
}


