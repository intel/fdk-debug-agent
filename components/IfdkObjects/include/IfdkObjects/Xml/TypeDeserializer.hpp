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

#pragma once

#include "IfdkObjects/Xml/TypeTraits.hpp"
#include "IfdkObjects/Xml/Deserializer.hpp"
#include "IfdkObjects/Type/Visitor.hpp"

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

 /* XML Deserializer for the "Type" data model.
  *
  * It implements the type::Visitor interface.
  */
class TypeDeserializer final : public Deserializer<TypeTraits>, public type::Visitor
{
public:
    TypeDeserializer(const std::string &xml) : Deserializer<TypeTraits>(xml) {}

private:

    /* Visitor interface implementation */
    virtual void enter(type::Type &instance, bool isConcrete) override;
    virtual void enter(type::Component &instance, bool isConcrete) override;
    virtual void enter(type::Subsystem &instance) override;
    virtual void enter(type::System &instance) override;
    virtual void enter(type::Categories &instance) override;
    virtual void enter(type::Ref &instance, bool isConcrete) override;
    virtual void enter(type::TypeRef &instance) override;
    virtual void enter(type::ComponentRef &instance) override;
    virtual void enter(type::ServiceRef &instance) override;
    virtual void enter(type::SubsystemRef &instance) override;
    virtual void enter(type::TypeRefCollection &instance) override;
    virtual void enter(type::ComponentRefCollection &instance) override;
    virtual void enter(type::ServiceRefCollection &instance) override;
    virtual void enter(type::SubsystemRefCollection &instance) override;
    virtual void enter(type::Children &instance) override;
    virtual void enter(type::RefCollection &instance, bool isConcrete) override;
    virtual void enter(type::Characteristic &instance) override;
    virtual void enter(type::Characteristics &instance) override;
    virtual void enter(type::Description &instance) override;
    virtual void enter(type::Parameters &instance, bool isConcrete) override;
    virtual void enter(type::InfoParameters &instance) override;
    virtual void enter(type::ControlParameters &instance) override;
    virtual void enter(type::Connector &instance, bool isConcrete) override;
    virtual void enter(type::Input &instance) override;
    virtual void enter(type::Output &instance) override;
    virtual void enter(type::Inputs &instance) override;
    virtual void enter(type::Outputs &instance) override;
    virtual void leave(bool isConcrete) override;

private:
    template <class T>
    void collectionCommon(type::GenericRefCollection<T> &collection);
};

}
}
}

