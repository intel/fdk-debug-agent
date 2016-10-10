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
 *
 * @code
 * 1. enter(Characteristics& list)  (the type "Characteristics" is a list of characteristics)
 * 2.   enter(Characteristic &characteristic) (the "a" instance)
 * 3.   leave()
 * 4.   enter(Characteristic &characteristic) (the "b" instance)
 * 5.   leave()
 * 6. leave()
 * @endcode
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
 * @code
 * class A
 * {
 *    memberA;
 * }
 *
 * class B : public A
 * {
 *    memberB;
 * }
 * @endcode
 *
 * If you visit an "A" instance, the visitor call sequence will be:
 *
 * @code
 * 1. enter(A &a, isConcrete = true) -> create the <A> xml tag, set the "memberA" attribute
 * 2. leave(isConcrete = true)
 * @endcode
 *
 * the resulting xml is:
 * @code
 * <A memberA="a"/>
 * @endcode
 *
 * If you visit a "B" instance, the visitor call sequence will be:
 *
 * @code
 * 1. enter(B &b, isConcrete = true) -> create the <B> xml tag, set the "memberB" attribute
 * 2.   enter(A &a, isConcrete = false) -> set the "memberA" attribute
 * 3.   leave(isConcrete = false)
 * 4. leave(isConcrete = true)
 * @endcode
 *
 * the resulting xml is:
 * @code
 * <B memberB="b" memberA="a"/>
 * @endcode
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
