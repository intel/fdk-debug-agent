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

#include "IfdkObjects/Xml/TypeTraits.hpp"
#include <map>
#include <string>
#include <memory>

namespace debug_agent
{
namespace core
{

/** Main class of instance data model */
class TypeModel
{
public:
    using TypePtr = std::shared_ptr<const ifdk_objects::type::Type>;
    using SystemPtr = std::shared_ptr<const ifdk_objects::type::System>;

    using TypeMap = std::map<std::string, TypePtr>;

    TypeModel(const SystemPtr &systemPtr,
        const TypeMap &typeMap) :
        mSystemPtr(systemPtr), mTypeMap(typeMap) {}

    /** @return the system type */
    const SystemPtr getSystem() const
    {
        return mSystemPtr;
    }

    /** @return a type by its name, or nullptr if not found */
    const TypePtr getType(const std::string &typeName) const
    {
        auto it = mTypeMap.find(typeName);
        if (it == mTypeMap.end()) {
            return nullptr;
        }
        return it->second;
    }

    const TypeMap &getTypeMap() const
    {
        return mTypeMap;
    }

private:
    TypeModel(const TypeModel&) = delete;
    TypeModel &operator=(const TypeModel&) = delete;

    SystemPtr mSystemPtr;
    TypeMap mTypeMap;
};

}
}


