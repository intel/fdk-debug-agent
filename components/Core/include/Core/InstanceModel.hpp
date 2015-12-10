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

#include "IfdkObjects/Xml/InstanceTraits.hpp"
#include <map>
#include <string>
#include <memory>
namespace debug_agent
{
namespace core
{

/** Main class of instance data model */
class InstanceModel
{
public:
    using CollectionPtr = std::shared_ptr<const ifdk_objects::instance::BaseCollection>;
    using InstancePtr = std::shared_ptr<const ifdk_objects::instance::Instance>;

    using CollectionMap = std::map<std::string, CollectionPtr>;

    InstanceModel(const CollectionMap &collectionMap) : mCollectionMap(collectionMap) {}

    /** @return a collection by its name, or nullptr if not found */
    const CollectionPtr getCollection(const std::string &typeName) const
    {
        auto it = mCollectionMap.find(typeName);
        if (it == mCollectionMap.end()) {
            return nullptr;
        }
        return it->second;
    }

    /** @return an instance by its type and instance id, or nullptr if not found */
    const InstancePtr getInstance(const std::string &typeName, const std::string &instanceId) const
    {
        CollectionPtr collection = getCollection(typeName);
        if (collection == nullptr) {
            return nullptr;
        }
        return collection->getInstance(instanceId);
    }

    const CollectionMap &getCollectionMap() const { return mCollectionMap; }

private:
    InstanceModel(const InstanceModel &) = delete;
    InstanceModel &operator=(const InstanceModel &) = delete;

    CollectionMap mCollectionMap;
};
}
}
