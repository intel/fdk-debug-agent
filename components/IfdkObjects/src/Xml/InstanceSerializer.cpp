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

#include "IfdkObjects/Xml/InstanceSerializer.hpp"
#include <assert.h>
#include <sstream>

using namespace Poco;
using namespace Poco::XML;
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

void InstanceSerializer::enter(const Parameters &parameters, bool isConcrete)
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

void InstanceSerializer::leave(bool isConcrete)
{
    if (isConcrete) {
        popElement();
    }
}

}
}
}


