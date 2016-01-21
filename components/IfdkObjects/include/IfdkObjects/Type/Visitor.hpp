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

#include "IfdkObjects/VisitableVector.hpp"
#include "IfdkObjects/VisitablePtrVector.hpp"

namespace debug_agent
{
namespace ifdk_objects
{
namespace type
{

/** Forward declaring visited classes, because according to the Visitor pattern, there is a
 * circular dependency between the visitor and the visited classes */

class Visitor;
class ConstVisitor;
class Ref;
class TypeRef;
class ComponentRef;
class ServiceRef;
class EndPointRef;
class SubsystemRef;
class Type;
class Component;
class Subsystem;
class System;
class Service;
class EndPoint;
class RefCollection;
class Characteristic;
class Description;
class Parameters;
class InfoParameters;
class ControlParameters;
class Connector;
class Input;
class Output;

/* Some visitied types are template instantiations, so forward declaring them. */
using Categories = VisitablePtrVector<Ref, Visitor, ConstVisitor>;
using Children = VisitablePtrVector<RefCollection, Visitor, ConstVisitor>;
using Characteristics = VisitableVector<Characteristic, Visitor, ConstVisitor>;
using Inputs = VisitableVector<Input, Visitor, ConstVisitor>;
using Outputs = VisitableVector<Output, Visitor, ConstVisitor>;

template <class T>
class GenericRefCollection;
using TypeRefCollection = GenericRefCollection<TypeRef>;
using ComponentRefCollection = GenericRefCollection<ComponentRef>;
using ServiceRefCollection = GenericRefCollection<ServiceRef>;
using EndPointRefCollection = GenericRefCollection<EndPointRef>;
using SubsystemRefCollection = GenericRefCollection<SubsystemRef>;

/** Visitor that allows to walk into the "type" data model
 *
 * It is a non const visitor, i.e. data model instances can be modified
 *
 * When a class is visited, the matching enter() method is called. Then subnodes are visited.
 * A finally, when all subnodes have been visited, the leave() method is called.
 *
 * Here is a example:
 *
 * Consider this structure:
 * - a characteristic list that contains
 *    - a characteristic "a"
 *    - another characteristic "b"
 *
 * - the matching visitor call sequence will be:
 * 1. enter(Characteristics& list)  (the type "Characteristics" is a list of characteristics)
 * 2.   enter(Characteristic &characteristic) (the "a" instance)
 * 3.   leave()
 * 4.   enter(Characteristic &characteristic) (the "b" instance)
 * 5.   leave()
 * 6. leave()
 *
 * Note: some enter() methods provides the "isConcrete" parameter. This parameter is used to know
 * if a class is visited as parent class or as concrete class.
 * This information is useful for instance for xml serialization:
 * - if the class is visited as concrete class, create a XML tag.
 * - if the class is visited as parent class, only set the matching attributes, the XML tag
 *   has already been created while visiting the concrete class.
 *
 * For instance consider this model:
 *
 * class A
 * {
 *    memberA;
 * }
 *
 * class B : public A
 * {
 *    memberB;
 * }
 *
 * If you visit an "A" instance, the visitor call sequence will be:
 * 1. enter(A &a, isConcrete = true) -> create the <A> xml tag, set the "memberA" attribute
 * 2. leave(isConcrete = true)
 *
 * the resulting xml is:
 * <A memberA="a"/>
 *
 * If you visit a "B" instance, the visitor call sequence will be:
 * 1. enter(B &b, isConcrete = true) -> create the <B> xml tag, set the "memberB" attribute
 * 2.   enter(A &a, isConcrete = false) -> set the "memberA" attribute
 * 3.   leave(isConcrete = false)
 * 4. leave(isConcrete = true)
 *
 * the resulting xml is:
 * <B memberB="b" memberA="a"/>
 */
class Visitor
{
public:
    virtual ~Visitor() = default;

    /* References */
    virtual void enter(Ref &instance, bool isConcrete) = 0;
    virtual void enter(TypeRef &instance) = 0;
    virtual void enter(ComponentRef &instance) = 0;
    virtual void enter(ServiceRef &instance) = 0;
    virtual void enter(EndPointRef &instance) = 0;
    virtual void enter(SubsystemRef &instance) = 0;

    /* Named reference collections */
    virtual void enter(RefCollection &instance, bool isConcrete) = 0;
    virtual void enter(TypeRefCollection &instance) = 0;
    virtual void enter(ComponentRefCollection &instance) = 0;
    virtual void enter(ServiceRefCollection &instance) = 0;
    virtual void enter(EndPointRefCollection &instance) = 0;
    virtual void enter(SubsystemRefCollection &instance) = 0;

    /* Characteristics */
    virtual void enter(Characteristic &instance) = 0;
    virtual void enter(Characteristics &instance) = 0;

    /* Misc types */
    virtual void enter(Description &instance) = 0;

    /* Misc collections */
    virtual void enter(Categories &instance) = 0;
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

    /* Main type classes */
    virtual void enter(Type &instance, bool isConcrete) = 0;
    virtual void enter(Component &instance, bool isConcrete) = 0;
    virtual void enter(Subsystem &instance) = 0;
    virtual void enter(System &instance) = 0;
    virtual void enter(Service &instance) = 0;
    virtual void enter(EndPoint &instance) = 0;

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
    virtual void enter(const TypeRef &instance) = 0;
    virtual void enter(const ComponentRef &instance) = 0;
    virtual void enter(const ServiceRef &instance) = 0;
    virtual void enter(const EndPointRef &instance) = 0;
    virtual void enter(const SubsystemRef &instance) = 0;

    /* Named reference collections */
    virtual void enter(const RefCollection &instance, bool isConcrete) = 0;
    virtual void enter(const TypeRefCollection &instance) = 0;
    virtual void enter(const ComponentRefCollection &instance) = 0;
    virtual void enter(const ServiceRefCollection &instance) = 0;
    virtual void enter(const EndPointRefCollection &instance) = 0;
    virtual void enter(const SubsystemRefCollection &instance) = 0;

    /* Characteristics */
    virtual void enter(const Characteristic &instance) = 0;
    virtual void enter(const Characteristics &instance) = 0;

    /* Misc types */
    virtual void enter(const Description &instance) = 0;

    /* Misc collections */
    virtual void enter(const Categories &instance) = 0;
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

    /* Main type classes */
    virtual void enter(const Type &instance, bool isConcrete) = 0;
    virtual void enter(const Component &instance, bool isConcrete) = 0;
    virtual void enter(const Subsystem &instance) = 0;
    virtual void enter(const System &instance) = 0;
    virtual void enter(const Service &instance) = 0;
    virtual void enter(const EndPoint &instance) = 0;

    /* Common 'leave' method */
    virtual void leave(bool isConcrete = true) = 0;
};
}
}
}
