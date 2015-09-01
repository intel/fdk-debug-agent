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
class SubsystemRef;
class SystemRef;
class Instance;
class Component;
class Subsystem;
class System;
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
using InstanceCollection = VisitablePtrVector<Instance, Visitor, ConstVisitor>;
using ComponentCollection = VisitablePtrVector<Component, Visitor, ConstVisitor>;
using SubsystemCollection = VisitablePtrVector<Subsystem, Visitor, ConstVisitor>;
using Parents = VisitablePtrVector<Ref, Visitor, ConstVisitor>;

template <class T>
class GenericRefCollection;
using InstanceRefCollection = GenericRefCollection<InstanceRef>;
using ComponentRefCollection = GenericRefCollection<ComponentRef>;
using ServiceRefCollection = GenericRefCollection<ServiceRef>;
using SubsystemRefCollection = GenericRefCollection<SubsystemRef>;

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
    virtual void enter(SubsystemRef &instance) = 0;
    virtual void enter(SystemRef &instance) = 0;

    /* Named reference collections */
    virtual void enter(RefCollection &instance, bool isConcrete) = 0;
    virtual void enter(InstanceRefCollection &instance) = 0;
    virtual void enter(ComponentRefCollection &instance) = 0;
    virtual void enter(ServiceRefCollection &instance) = 0;
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

    /* Main instance collections */
    virtual void enter(InstanceCollection &instance) = 0;
    virtual void enter(ComponentCollection &instance) = 0;
    virtual void enter(SubsystemCollection &instance) = 0;

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
    virtual void enter(const SubsystemRef &instance) = 0;
    virtual void enter(const SystemRef &instance) = 0;

    /* Named reference collections */
    virtual void enter(const RefCollection &instance, bool isConcrete) = 0;
    virtual void enter(const InstanceRefCollection &instance) = 0;
    virtual void enter(const ComponentRefCollection &instance) = 0;
    virtual void enter(const ServiceRefCollection &instance) = 0;
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

    /* Main instance collections */
    virtual void enter(const InstanceCollection &instance) = 0;
    virtual void enter(const ComponentCollection &instance) = 0;
    virtual void enter(const SubsystemCollection &instance) = 0;

    /* Common 'leave' method */
    virtual void leave(bool isConcrete = true) = 0;
};

}
}
}


