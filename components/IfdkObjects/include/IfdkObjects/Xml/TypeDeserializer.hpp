/*
 * Copyright (c) 2015-2016, Intel Corporation
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
    virtual void enter(type::Service &instance) override;
    virtual void enter(type::EndPoint &instance) override;
    virtual void enter(type::Categories &instance) override;
    virtual void enter(type::Ref &instance, bool isConcrete) override;
    virtual void enter(type::TypeRef &instance) override;
    virtual void enter(type::ComponentRef &instance) override;
    virtual void enter(type::ServiceRef &instance) override;
    virtual void enter(type::EndPointRef &instance) override;
    virtual void enter(type::SubsystemRef &instance) override;
    virtual void enter(type::TypeRefCollection &instance) override;
    virtual void enter(type::ComponentRefCollection &instance) override;
    virtual void enter(type::SubsystemRefCollection &instance) override;
    virtual void enter(type::ServiceRefCollection &instance) override;
    virtual void enter(type::EndPointRefCollection &instance) override;
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
