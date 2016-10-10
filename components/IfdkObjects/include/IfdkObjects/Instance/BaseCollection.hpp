/*
 * Copyright (c) 2015, Intel Corporation
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

#include "IfdkObjects/Instance/Instance.hpp"
#include <vector>
#include <cassert>

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
    virtual std::shared_ptr<const Instance> getInstance(const std::string &instanceId) const = 0;

    /** @return all instances */
    virtual void getInstances(std::vector<std::shared_ptr<const Instance>> &list) const = 0;

    bool operator==(const BaseCollection &other) const noexcept
    {
        if (typeid(*this) != typeid(other)) {
            return false;
        }

        return equalsTo(other);
    }

protected:
    virtual bool equalsTo(const BaseCollection &other) const noexcept = 0;
};
}
}
}
