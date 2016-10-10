/*
 * Copyright (c) 2016, Intel Corporation
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
    virtual void enter(const instance::EndPointRef &instance) override;
    virtual void enter(const instance::SubsystemRef &instance) override;
    virtual void enter(const instance::SystemRef &instance) override;

    /* Named reference collections */
    virtual void enter(const instance::RefCollection &instance, bool isConcrete) override;
    virtual void enter(const instance::InstanceRefCollection &instance) override;
    virtual void enter(const instance::ComponentRefCollection &instance) override;
    virtual void enter(const instance::ServiceRefCollection &instance) override;
    virtual void enter(const instance::EndPointRefCollection &instance) override;
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
    virtual void enter(const instance::EndPoint &instance) override;

    /* Main instance collections */
    virtual void enter(const instance::InstanceCollection &instance) override;
    virtual void enter(const instance::ComponentCollection &instance) override;
    virtual void enter(const instance::SubsystemCollection &instance) override;
    virtual void enter(const instance::ServiceCollection &instance) override;
    virtual void enter(const instance::EndPointCollection &instance) override;

    /* Common 'leave' method */
    virtual void leave(bool isConcrete = true) override;
};
}
}
}
