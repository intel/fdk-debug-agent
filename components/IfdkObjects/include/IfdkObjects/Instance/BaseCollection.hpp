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
#include <vector>
#include <assert.h>

namespace debug_agent
{
namespace ifdk_objects
{
namespace instance
{

/**
 * Base class of instance collections
 *
 * Instance collections are not polymorphic, they are collections of one instance type only:
 * Collection<Instance>
 * Collection<Component>
 * Collection<Subsystem>
 *
 * Although Component and Subsystem classes derive from Instance class, it's not possible to
 * retrieve easily each element as instance.
 *
 * That's why theses collection will inherit from this base class, that allows access to contained
 * instances.
 */
class BaseCollection
{
public:
    virtual ~BaseCollection() = default;

    virtual void accept(Visitor &visitor, bool isConcrete = true) = 0;
    virtual void accept(ConstVisitor &visitor, bool isConcrete = true) const = 0;

    /** @return the instance that matches the supplied instanceId, or nullptr if no instance is
     * found.
     */
    virtual std::shared_ptr<Instance> getInstance(const std::string &instanceId) = 0;

    bool operator == (const BaseCollection &other) const NOEXCEPT
    {
        if (typeid(*this) != typeid(other)) {
            return false;
        }

        return equalsTo(other);
    }

protected:
    virtual bool equalsTo(const BaseCollection &other) const NOEXCEPT = 0;

};

}
}
}


