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

void TypeSerializer::enter(const Parameters &parameters, bool isConcrete)
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


