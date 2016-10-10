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

#include "IfdkObjects/VisitableVector.hpp"
#include "IfdkObjects/VisitablePtrVector.hpp"

namespace debug_agent
{
namespace ifdk_objects
{
namespace instance
{

/** Forward declaring visited classes, because according to the Visitor pattern, there is a
 * circular dependency between the visitor and the visited classes */

class Visitor;
class ConstVisitor;
class Ref;
class InstanceRef;
class ComponentRef;
class ServiceRef;
class EndPointRef;
class SubsystemRef;
class SystemRef;
class Instance;
class Component;
class Subsystem;
class System;
class Service;
class EndPoint;
class RefCollection;
class Description;
class Parameters;
class InfoParameters;
class ControlParameters;
class Connector;
class Input;
class Output;
class From;
class To;
class Link;

/* Some visitied types are template instantiations, so forward declaring them. */
using Children = VisitablePtrVector<RefCollection, Visitor, ConstVisitor>;
using Inputs = VisitableVector<Input, Visitor, ConstVisitor>;
using Outputs = VisitableVector<Output, Visitor, ConstVisitor>;
using Links = VisitableVector<Link, Visitor, ConstVisitor>;
using Parents = VisitablePtrVector<Ref, Visitor, ConstVisitor>;

template <class T>
class GenericRefCollection;
using InstanceRefCollection = GenericRefCollection<InstanceRef>;
using ComponentRefCollection = GenericRefCollection<ComponentRef>;
using ServiceRefCollection = GenericRefCollection<ServiceRef>;
using EndPointRefCollection = GenericRefCollection<EndPointRef>;
using SubsystemRefCollection = GenericRefCollection<SubsystemRef>;

template <class T>
class GenericCollection;
using InstanceCollection = GenericCollection<Instance>;
using ComponentCollection = GenericCollection<Component>;
using SubsystemCollection = GenericCollection<Subsystem>;
using ServiceCollection = GenericCollection<Service>;
using EndPointCollection = GenericCollection<EndPoint>;

/** Visitor that allows to walk into the "instance" data model
 *
 * It is a non const visitor, i.e. data model instances can be modified
 *
 * @see type::Visitor for more detail about visitor usage.
 */
class Visitor
{
public:
    virtual ~Visitor() = default;

    /* References */
    virtual void enter(Ref &instance, bool isConcrete) = 0;
    virtual void enter(InstanceRef &instance) = 0;
    virtual void enter(ComponentRef &instance) = 0;
    virtual void enter(ServiceRef &instance) = 0;
    virtual void enter(EndPointRef &instance) = 0;
    virtual void enter(SubsystemRef &instance) = 0;
    virtual void enter(SystemRef &instance) = 0;

    /* Named reference collections */
    virtual void enter(RefCollection &instance, bool isConcrete) = 0;
    virtual void enter(InstanceRefCollection &instance) = 0;
    virtual void enter(ComponentRefCollection &instance) = 0;
    virtual void enter(ServiceRefCollection &instance) = 0;
    virtual void enter(EndPointRefCollection &instance) = 0;
    virtual void enter(SubsystemRefCollection &instance) = 0;

    /* Parents and children */
    virtual void enter(Parents &instance) = 0;
    virtual void enter(Children &instance) = 0;

    /* Parameters */
    virtual void enter(Parameters &instance, bool isConcrete) = 0;
    virtual void enter(InfoParameters &instance) = 0;
    virtual void enter(ControlParameters &instance) = 0;

    /* Inputs / Outputs */
    virtual void enter(Connector &instance, bool isConcrete) = 0;
    virtual void enter(Input &instance) = 0;
    virtual void enter(Output &instance) = 0;
    virtual void enter(Inputs &instance) = 0;
    virtual void enter(Outputs &instance) = 0;

    /* Links */
    virtual void enter(To &instance) = 0;
    virtual void enter(From &instance) = 0;
    virtual void enter(Link &instance) = 0;
    virtual void enter(Links &instance) = 0;

    /* Main instance classes */
    virtual void enter(Instance &instance, bool isConcrete) = 0;
    virtual void enter(Component &instance, bool isConcrete) = 0;
    virtual void enter(Subsystem &instance) = 0;
    virtual void enter(System &instance) = 0;
    virtual void enter(Service &instance) = 0;
    virtual void enter(EndPoint &instance) = 0;

    /* Main instance collections */
    virtual void enter(InstanceCollection &instance) = 0;
    virtual void enter(ComponentCollection &instance) = 0;
    virtual void enter(SubsystemCollection &instance) = 0;
    virtual void enter(ServiceCollection &instance) = 0;
    virtual void enter(EndPointCollection &instance) = 0;

    /* Common 'leave' method */
    virtual void leave(bool isConcrete = true) = 0;
};

/** This visitor walks on const instances */
class ConstVisitor
{
public:
    virtual ~ConstVisitor() = default;

    /* References */
    virtual void enter(const Ref &instance, bool isConcrete) = 0;
    virtual void enter(const InstanceRef &instance) = 0;
    virtual void enter(const ComponentRef &instance) = 0;
    virtual void enter(const ServiceRef &instance) = 0;
    virtual void enter(const EndPointRef &instance) = 0;
    virtual void enter(const SubsystemRef &instance) = 0;
    virtual void enter(const SystemRef &instance) = 0;

    /* Named reference collections */
    virtual void enter(const RefCollection &instance, bool isConcrete) = 0;
    virtual void enter(const InstanceRefCollection &instance) = 0;
    virtual void enter(const ComponentRefCollection &instance) = 0;
    virtual void enter(const ServiceRefCollection &instance) = 0;
    virtual void enter(const EndPointRefCollection &instance) = 0;
    virtual void enter(const SubsystemRefCollection &instance) = 0;

    /* Parents and children */
    virtual void enter(const Parents &instance) = 0;
    virtual void enter(const Children &instance) = 0;

    /* Parameters */
    virtual void enter(const Parameters &instance, bool isConcrete) = 0;
    virtual void enter(const InfoParameters &instance) = 0;
    virtual void enter(const ControlParameters &instance) = 0;

    /* Inputs / Outputs */
    virtual void enter(const Connector &instance, bool isConcrete) = 0;
    virtual void enter(const Input &instance) = 0;
    virtual void enter(const Output &instance) = 0;
    virtual void enter(const Inputs &instance) = 0;
    virtual void enter(const Outputs &instance) = 0;

    /* Links */
    virtual void enter(const To &instance) = 0;
    virtual void enter(const From &instance) = 0;
    virtual void enter(const Link &instance) = 0;
    virtual void enter(const Links &instance) = 0;

    /* Main instance classes */
    virtual void enter(const Instance &instance, bool isConcrete) = 0;
    virtual void enter(const Component &instance, bool isConcrete) = 0;
    virtual void enter(const Subsystem &instance) = 0;
    virtual void enter(const System &instance) = 0;
    virtual void enter(const Service &instance) = 0;
    virtual void enter(const EndPoint &instance) = 0;

    /* Main instance collections */
    virtual void enter(const InstanceCollection &instance) = 0;
    virtual void enter(const ComponentCollection &instance) = 0;
    virtual void enter(const SubsystemCollection &instance) = 0;
    virtual void enter(const ServiceCollection &instance) = 0;
    virtual void enter(const EndPointCollection &instance) = 0;

    /* Common 'leave' method */
    virtual void leave(bool isConcrete = true) = 0;
};
}
}
}
