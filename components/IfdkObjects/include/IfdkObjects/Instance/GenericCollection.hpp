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

#include "IfdkObjects/Instance/BaseCollection.hpp"
#include <vector>
#include <memory>
#include <cassert>

namespace debug_agent
{
namespace ifdk_objects
{
namespace instance
{

/**
 * Generic instance collection
 *
 * Instance collections are not polymorphic, they are collections of one instance type only.
 * This class allows to make this kind of collection.
 */
template <class T>
class GenericCollection : public BaseCollection
{
private:
    using base = BaseCollection;

public:
    GenericCollection() = default;
    explicit GenericCollection(const GenericCollection &other) = default;
    GenericCollection &operator=(const GenericCollection &other) = default;

    virtual void accept(Visitor &visitor, bool isConcrete = true) override
    {
        assert(isConcrete);
        acceptCommon(*this, visitor);
    }

    virtual void accept(ConstVisitor &visitor, bool isConcrete = true) const override
    {
        assert(isConcrete);
        acceptCommon(*this, visitor);
    }

    virtual std::shared_ptr<const Instance> getInstance(
        const std::string &instanceId) const override
    {
        for (auto elementPtr : mElementPtrs) {
            if (elementPtr->getInstanceId() == instanceId) {
                return elementPtr;
            }
        }

        return nullptr;
    }

    virtual void getInstances(std::vector<std::shared_ptr<const Instance>> &list) const override
    {
        for (auto elementPtr : mElementPtrs) {
            list.push_back(elementPtr);
        }
    }

    void add(T *element) { add(std::shared_ptr<T>(element)); }

    void add(std::shared_ptr<T> element)
    {
        assert(element != nullptr);
        mElementPtrs.push_back(element);
    }

protected:
    virtual bool equalsTo(const BaseCollection &other) const noexcept override
    {
        const GenericCollection<T> *otherColl = dynamic_cast<const GenericCollection<T> *>(&other);
        if (otherColl == nullptr) {
            return false;
        }

        if (mElementPtrs.size() != otherColl->mElementPtrs.size()) {
            return false;
        }

        for (std::size_t i = 0; i < mElementPtrs.size(); ++i) {
            if (*mElementPtrs[i] != *otherColl->mElementPtrs[i]) {
                return false;
            }
        }

        return true;
    }

private:
    using ElementPtrVector = std::vector<std::shared_ptr<T>>;

    template <typename MyInstance, typename Visitor>
    static void acceptCommon(MyInstance &me, Visitor &visitor)
    {
        visitor.enter(me);

        for (auto &element : me.mElementPtrs) {
            element->accept(visitor);
        }

        visitor.leave();
    }

    ElementPtrVector mElementPtrs;
};
}
}
}
