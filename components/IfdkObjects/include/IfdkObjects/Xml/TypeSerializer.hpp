/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(const type::type::C) 2015 Intel Corporation. All Rights Reserved.
*   The source code contained  or  described herein and all documents related to
*   the source code (const type::type::"Material") are owned by Intel Corporation or its suppliers
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
#include "IfdkObjects/Xml/Serializer.hpp"
#include "IfdkObjects/Type/Visitor.hpp"

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

/* XML Serializer for the "Type" data model.
 *
 * It implements the type::ConstVisitor interface.
 */
class TypeSerializer final : public Serializer<TypeTraits>, public type::ConstVisitor
{
public:
    TypeSerializer() = default;

private:
    /* ConstVisitor interface implementation */
    virtual void enter(const type::Type &instance, bool isConcrete) override;
    virtual void enter(const type::Component &instance, bool isConcrete) override;
    virtual void enter(const type::Subsystem &instance) override;
    virtual void enter(const type::System &instance) override;
    virtual void enter(const type::Service &instance) override;
    virtual void enter(const type::Categories &instance) override;
    virtual void enter(const type::Ref &instance, bool isConcrete);
    virtual void enter(const type::TypeRef &instance) override;
    virtual void enter(const type::ComponentRef &instance) override;
    virtual void enter(const type::ServiceRef &instance) override;
    virtual void enter(const type::SubsystemRef &instance) override;
    virtual void enter(const type::TypeRefCollection &instance) override;
    virtual void enter(const type::ComponentRefCollection &instance) override;
    virtual void enter(const type::ServiceRefCollection &instance) override;
    virtual void enter(const type::SubsystemRefCollection &instance) override;
    virtual void enter(const type::Children &instance) override;
    virtual void enter(const type::RefCollection &instance, bool isConcrete) override;
    virtual void enter(const type::Characteristic &instance) override;
    virtual void enter(const type::Characteristics &instance) override;
    virtual void enter(const type::Description &instance) override;
    virtual void enter(const type::Parameters &instance, bool isConcrete) override;
    virtual void enter(const type::InfoParameters &instance) override;
    virtual void enter(const type::ControlParameters &instance) override;
    virtual void enter(const type::Connector &instance, bool isConcrete) override;
    virtual void enter(const type::Input &instance) override;
    virtual void enter(const type::Output &instance) override;
    virtual void enter(const type::Inputs &instance) override;
    virtual void enter(const type::Outputs &instance) override;
    virtual void leave(bool isConcrete) override;
};
}
}
}
