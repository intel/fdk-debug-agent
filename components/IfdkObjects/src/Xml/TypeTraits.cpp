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

#include "IfdkObjects/Xml/TypeTraits.hpp"

using namespace debug_agent::ifdk_objects::type;

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

const std::string TypeTraits<Ref>::attributeName("Name");

const std::string TypeTraits<TypeRef>::tag("type");

const std::string TypeTraits<ComponentRef>::tag("component_type");

const std::string TypeTraits<ServiceRef>::tag("service_type");

const std::string TypeTraits<EndPointRef>::tag("endpoint_type");

const std::string TypeTraits<SubsystemRef>::tag("subsystem_type");

const std::string TypeTraits<Type>::tag("type");
const std::string TypeTraits<Type>::attributeName("Name");

const std::string TypeTraits<Component>::tag("component_type");

const std::string TypeTraits<Subsystem>::tag("subsystem_type");

const std::string TypeTraits<System>::tag("system_type");

const std::string TypeTraits<Service>::tag("service_type");

const std::string TypeTraits<EndPoint>::tag("endpoint_type");
const std::string TypeTraits<EndPoint>::attributeDirection("Direction");

const std::string TypeTraits<Categories>::tag("categories");

const std::string TypeTraits<Children>::tag("children");

const std::string TypeTraits<RefCollection>::attributeName("Name");

const std::string TypeTraits<TypeRefCollection>::tag("collection");

const std::string TypeTraits<ComponentRefCollection>::tag("component_collection");

const std::string TypeTraits<ServiceRefCollection>::tag("service_collection");

const std::string TypeTraits<EndPointRefCollection>::tag("endpoint_collection");

const std::string TypeTraits<SubsystemRefCollection>::tag("subsystem_collection");

const std::string TypeTraits<Characteristic>::tag("characteristic");
const std::string TypeTraits<Characteristic>::attributeName("Name");

const std::string TypeTraits<Characteristics>::tag("characteristics");

const std::string TypeTraits<Description>::tag("description");

const std::string TypeTraits<InfoParameters>::tag("info_parameters");

const std::string TypeTraits<ControlParameters>::tag("control_parameters");

const std::string TypeTraits<Connector>::attributeId("Id");
const std::string TypeTraits<Connector>::attributeName("Name");

const std::string TypeTraits<Input>::tag("input");

const std::string TypeTraits<Output>::tag("output");

const std::string TypeTraits<Inputs>::tag("inputs");

const std::string TypeTraits<Outputs>::tag("outputs");
}
}
}
