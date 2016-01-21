/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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

#pragma once

#include "IfdkObjects/Xml/InstanceTraits.hpp"
#include "IfdkObjects/Xml/Deserializer.hpp"
#include "IfdkObjects/Instance/Visitor.hpp"

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

/* XML Deserializer for the "Instance" data model.
 *
 * It implements the instance::Visitor interface.
 */
class InstanceDeserializer final : public Deserializer<InstanceTraits>, public instance::Visitor
{
public:
    InstanceDeserializer(const std::string &xml) : Deserializer<InstanceTraits>(xml) {}

private:
    /* References */
    virtual void enter(instance::Ref &instance, bool isConcrete) override;
    virtual void enter(instance::InstanceRef &instance) override;
    virtual void enter(instance::ComponentRef &instance) override;
    virtual void enter(instance::ServiceRef &instance) override;
    virtual void enter(instance::EndPointRef &instance) override;
    virtual void enter(instance::SubsystemRef &instance) override;
    virtual void enter(instance::SystemRef &instance) override;

    /* Named reference collections */
    virtual void enter(instance::RefCollection &instance, bool isConcrete) override;
    virtual void enter(instance::InstanceRefCollection &instance) override;
    virtual void enter(instance::ComponentRefCollection &instance) override;
    virtual void enter(instance::ServiceRefCollection &instance) override;
    virtual void enter(instance::EndPointRefCollection &instance) override;
    virtual void enter(instance::SubsystemRefCollection &instance) override;

    /* Parents and children */
    virtual void enter(instance::Parents &instance) override;
    virtual void enter(instance::Children &instance) override;

    /* Parameters */
    virtual void enter(instance::Parameters &instance, bool isConcrete) override;
    virtual void enter(instance::InfoParameters &instance) override;
    virtual void enter(instance::ControlParameters &instance) override;

    /* Inputs / Outputs */
    virtual void enter(instance::Connector &instance, bool isConcrete) override;
    virtual void enter(instance::Input &instance) override;
    virtual void enter(instance::Output &instance) override;
    virtual void enter(instance::Inputs &instance) override;
    virtual void enter(instance::Outputs &instance) override;

    /* Links */
    virtual void enter(instance::To &instance) override;
    virtual void enter(instance::From &instance) override;
    virtual void enter(instance::Link &instance) override;
    virtual void enter(instance::Links &instance) override;

    /* Main instance classes */
    virtual void enter(instance::Instance &instance, bool isConcrete) override;
    virtual void enter(instance::Component &instance, bool isConcrete) override;
    virtual void enter(instance::Subsystem &instance) override;
    virtual void enter(instance::System &instance) override;
    virtual void enter(instance::Service &instance) override;
    virtual void enter(instance::EndPoint &instance) override;

    /* Main instance collections */
    virtual void enter(instance::InstanceCollection &instance) override;
    virtual void enter(instance::ComponentCollection &instance) override;
    virtual void enter(instance::SubsystemCollection &instance) override;
    virtual void enter(instance::ServiceCollection &instance) override;
    virtual void enter(instance::EndPointCollection &instance) override;

    /* Common 'leave' method */
    virtual void leave(bool isConcrete = true) override;

private:
    template <class T>
    void refCollectionCommon(instance::GenericRefCollection<T> &collection);

    template <class T>
    void collectionCommon(instance::GenericCollection<T> &collection);
};
}
}
}
