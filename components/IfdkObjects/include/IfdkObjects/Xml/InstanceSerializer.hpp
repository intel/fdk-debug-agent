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

#include "IfdkObjects/Xml/InstanceTraits.hpp"
#include "IfdkObjects/Xml/Serializer.hpp"
#include "IfdkObjects/Instance/Visitor.hpp"

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
class InstanceSerializer final : public Serializer<InstanceTraits>, public instance::ConstVisitor
{
public:
    InstanceSerializer() = default;

private:
    /* ConstVisitor interface implementation */

    /* References */
    virtual void enter(const instance::Ref &instance, bool isConcrete) override;
    virtual void enter(const instance::InstanceRef &instance) override;
    virtual void enter(const instance::ComponentRef &instance) override;
    virtual void enter(const instance::ServiceRef &instance) override;
    virtual void enter(const instance::SubsystemRef &instance) override;
    virtual void enter(const instance::SystemRef &instance) override;

    /* Named reference collections */
    virtual void enter(const instance::RefCollection &instance, bool isConcrete) override;
    virtual void enter(const instance::InstanceRefCollection &instance) override;
    virtual void enter(const instance::ComponentRefCollection &instance) override;
    virtual void enter(const instance::ServiceRefCollection &instance) override;
    virtual void enter(const instance::SubsystemRefCollection &instance) override;

    /* Parents and children */
    virtual void enter(const instance::Parents &instance) override;
    virtual void enter(const instance::Children &instance) override;

    /* Parameters */
    virtual void enter(const instance::Parameters &instance, bool isConcrete) override;
    virtual void enter(const instance::InfoParameters &instance) override;
    virtual void enter(const instance::ControlParameters &instance) override;

    /* Inputs / Outputs */
    virtual void enter(const instance::Connector &instance, bool isConcrete) override;
    virtual void enter(const instance::Input &instance) override;
    virtual void enter(const instance::Output &instance) override;
    virtual void enter(const instance::Inputs &instance) override;
    virtual void enter(const instance::Outputs &instance) override;

    /* Links */
    virtual void enter(const instance::To &instance) override;
    virtual void enter(const instance::From &instance) override;
    virtual void enter(const instance::Link &instance) override;
    virtual void enter(const instance::Links &instance) override;

    /* Main instance classes */
    virtual void enter(const instance::Instance &instance, bool isConcrete) override;
    virtual void enter(const instance::Component &instance, bool isConcrete) override;
    virtual void enter(const instance::Subsystem &instance) override;
    virtual void enter(const instance::System &instance) override;
    virtual void enter(const instance::Service &instance) override;

    /* Main instance collections */
    virtual void enter(const instance::InstanceCollection &instance) override;
    virtual void enter(const instance::ComponentCollection &instance) override;
    virtual void enter(const instance::SubsystemCollection &instance) override;
    virtual void enter(const instance::ServiceCollection &instance) override;

    /* Common 'leave' method */
    virtual void leave(bool isConcrete = true) override;
};
}
}
}
