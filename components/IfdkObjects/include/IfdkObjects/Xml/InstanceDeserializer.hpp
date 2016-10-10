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
