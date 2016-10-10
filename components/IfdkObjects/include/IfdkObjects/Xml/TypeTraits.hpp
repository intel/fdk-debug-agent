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

#include "IfdkObjects/Type/Type.hpp"
#include "IfdkObjects/Type/Component.hpp"
#include "IfdkObjects/Type/Subsystem.hpp"
#include "IfdkObjects/Type/System.hpp"
#include "IfdkObjects/Type/Service.hpp"
#include "IfdkObjects/Type/EndPoint.hpp"
#include "IfdkObjects/Type/Categories.hpp"
#include "IfdkObjects/Type/TypeRef.hpp"
#include "IfdkObjects/Type/ComponentRef.hpp"
#include "IfdkObjects/Type/ServiceRef.hpp"
#include "IfdkObjects/Type/EndPointRef.hpp"
#include "IfdkObjects/Type/SubsystemRef.hpp"
#include "IfdkObjects/Type/Children.hpp"
#include "IfdkObjects/Type/TypeRefCollection.hpp"
#include "IfdkObjects/Type/ComponentRefCollection.hpp"
#include "IfdkObjects/Type/ServiceRefCollection.hpp"
#include "IfdkObjects/Type/EndPointRefCollection.hpp"
#include "IfdkObjects/Type/SubsystemRefCollection.hpp"
#include "IfdkObjects/Type/Characteristic.hpp"
#include "IfdkObjects/Type/Characteristics.hpp"
#include "IfdkObjects/Type/Description.hpp"
#include "IfdkObjects/Type/Connector.hpp"
#include "IfdkObjects/Type/Input.hpp"
#include "IfdkObjects/Type/Inputs.hpp"
#include "IfdkObjects/Type/Output.hpp"
#include "IfdkObjects/Type/Outputs.hpp"
#include "IfdkObjects/Type/Parameters.hpp"
#include "IfdkObjects/Type/InfoParameters.hpp"
#include "IfdkObjects/Type/ControlParameters.hpp"

#include <string>

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

/** Traits of the "Type" data model describing XML tags and attributes */
/** @{ */

/** base Traits class*/
template <class T>
struct TypeTraits
{
};

/* References */

template <>
struct TypeTraits<type::Ref>
{
    /* No tag because this class cannot be serializable */
    static const std::string attributeName;
};

template <>
struct TypeTraits<type::TypeRef>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::ComponentRef>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::ServiceRef>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::EndPointRef>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::SubsystemRef>
{
    static const std::string tag;
};

/* Named reference collections */

template <>
struct TypeTraits<type::RefCollection>
{
    /* No tag because this class cannot be serializable */
    static const std::string attributeName;
};

template <>
struct TypeTraits<type::TypeRefCollection>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::ComponentRefCollection>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::ServiceRefCollection>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::EndPointRefCollection>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::SubsystemRefCollection>
{
    static const std::string tag;
};

/* Characteristics */

template <>
struct TypeTraits<type::Characteristic>
{
    static const std::string tag;
    static const std::string attributeName;
};

template <>
struct TypeTraits<type::Characteristics>
{
    static const std::string tag;
};

/* Misc types */

template <>
struct TypeTraits<type::Description>
{
    static const std::string tag;
};

/* Misc collections */

template <>
struct TypeTraits<type::Categories>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::Children>
{
    static const std::string tag;
};

/* Parameters */

template <>
struct TypeTraits<type::Parameters>
{
    /* No tag because this class cannot be serializable */
};

template <>
struct TypeTraits<type::InfoParameters>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::ControlParameters>
{
    static const std::string tag;
};

/* Inputs / Outputs */

template <>
struct TypeTraits<type::Connector>
{
    /* No tag because this class cannot be serializable */
    static const std::string attributeId;
    static const std::string attributeName;
};

template <>
struct TypeTraits<type::Input>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::Output>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::Inputs>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::Outputs>
{
    static const std::string tag;
};

/* Main type classes */

template <>
struct TypeTraits<type::Type>
{
    static const std::string tag;
    static const std::string attributeName;
};

template <>
struct TypeTraits<type::Component>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::Subsystem>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::System>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::Service>
{
    static const std::string tag;
};

template <>
struct TypeTraits<type::EndPoint>
{
    static const std::string tag;
    static const std::string attributeDirection;
};
/** @} */
}
}
}
