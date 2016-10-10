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
    virtual void enter(const type::EndPoint &instance) override;
    virtual void enter(const type::Categories &instance) override;
    virtual void enter(const type::Ref &instance, bool isConcrete) override;
    virtual void enter(const type::TypeRef &instance) override;
    virtual void enter(const type::ComponentRef &instance) override;
    virtual void enter(const type::ServiceRef &instance) override;
    virtual void enter(const type::EndPointRef &instance) override;
    virtual void enter(const type::SubsystemRef &instance) override;
    virtual void enter(const type::TypeRefCollection &instance) override;
    virtual void enter(const type::ComponentRefCollection &instance) override;
    virtual void enter(const type::ServiceRefCollection &instance) override;
    virtual void enter(const type::EndPointRefCollection &instance) override;
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
