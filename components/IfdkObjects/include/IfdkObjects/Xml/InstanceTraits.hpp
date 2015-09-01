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

#include "IfdkObjects/Instance/Instance.hpp"
#include "IfdkObjects/Instance/Component.hpp"
#include "IfdkObjects/Instance/Subsystem.hpp"
#include "IfdkObjects/Instance/System.hpp"
#include "IfdkObjects/Instance/Parents.hpp"
#include "IfdkObjects/Instance/InstanceRef.hpp"
#include "IfdkObjects/Instance/ComponentRef.hpp"
#include "IfdkObjects/Instance/ServiceRef.hpp"
#include "IfdkObjects/Instance/SubsystemRef.hpp"
#include "IfdkObjects/Instance/SystemRef.hpp"
#include "IfdkObjects/Instance/Children.hpp"
#include "IfdkObjects/Instance/InstanceRefCollection.hpp"
#include "IfdkObjects/Instance/ComponentRefCollection.hpp"
#include "IfdkObjects/Instance/ServiceRefCollection.hpp"
#include "IfdkObjects/Instance/SubsystemRefCollection.hpp"
#include "IfdkObjects/Instance/Connector.hpp"
#include "IfdkObjects/Instance/Input.hpp"
#include "IfdkObjects/Instance/Inputs.hpp"
#include "IfdkObjects/Instance/Output.hpp"
#include "IfdkObjects/Instance/Outputs.hpp"
#include "IfdkObjects/Instance/Parameters.hpp"
#include "IfdkObjects/Instance/InfoParameters.hpp"
#include "IfdkObjects/Instance/ControlParameters.hpp"
#include "IfdkObjects/Instance/To.hpp"
#include "IfdkObjects/Instance/From.hpp"
#include "IfdkObjects/Instance/Link.hpp"
#include "IfdkObjects/Instance/Links.hpp"
#include "IfdkObjects/Instance/InstanceCollection.hpp"
#include "IfdkObjects/Instance/ComponentCollection.hpp"
#include "IfdkObjects/Instance/SubsystemCollection.hpp"

#include <string>

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

/** Traits of the "Instance" data model describing XML tags and attributes */

/** base Traits class*/
template <class T>
struct InstanceTraits
{
};

/* References */

template<>
struct InstanceTraits<instance::Ref>
{
    /* No tag because this class cannot be serializable */
    static const std::string attributeTypeName;
    static const std::string attributeInstanceId;
};

template<>
struct InstanceTraits<instance::InstanceRef>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::ComponentRef>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::ServiceRef>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::SubsystemRef>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::SystemRef>
{
    static const std::string tag;
};


/* Named reference collections */

template<>
struct InstanceTraits<instance::RefCollection>
{
    /* No tag because this class cannot be serializable */
    static const std::string attributeName;
};

template<>
struct InstanceTraits<instance::InstanceRefCollection>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::ComponentRefCollection>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::ServiceRefCollection>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::SubsystemRefCollection>
{
    static const std::string tag;
};

/* Parents & Children */

template<>
struct InstanceTraits<instance::Parents>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::Children>
{
    static const std::string tag;
};

/* Parameters */

template<>
struct InstanceTraits<instance::Parameters>
{
    /* No tag because this class cannot be serializable */
};

template<>
struct InstanceTraits<instance::InfoParameters>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::ControlParameters>
{
    static const std::string tag;
};

/* Inputs / Outputs */

template<>
struct InstanceTraits<instance::Connector>
{
    /* No tag because this class cannot be serializable */
    static const std::string attributeId;
    static const std::string attributeFormat;
};

template<>
struct InstanceTraits<instance::Input>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::Output>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::Inputs>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::Outputs>
{
    static const std::string tag;
};

/* Links*/

template<>
struct InstanceTraits<instance::To>
{
    static const std::string tag;
    static const std::string attributeTypeName;
    static const std::string attributeInstanceId;
    static const std::string attributeInputId;
};

template<>
struct InstanceTraits<instance::From>
{
    static const std::string tag;
    static const std::string attributeTypeName;
    static const std::string attributeInstanceId;
    static const std::string attributeOutputId;
};

template<>
struct InstanceTraits<instance::Link>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::Links>
{
    static const std::string tag;
};

/* Main type classes */

template<>
struct InstanceTraits<instance::Instance>
{
    static const std::string tag;
    static const std::string attributeTypeName;
    static const std::string attributeInstanceId;
};

template<>
struct InstanceTraits<instance::Component>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::Subsystem>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::System>
{
    static const std::string tag;
};

/* Main instance collection */

template<>
struct InstanceTraits<instance::InstanceCollection>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::ComponentCollection>
{
    static const std::string tag;
};

template<>
struct InstanceTraits<instance::SubsystemCollection>
{
    static const std::string tag;
};

}
}
}


