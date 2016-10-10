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

#include "IfdkObjects/Instance/RefCollection.hpp"
#include <vector>
#include <cassert>

namespace debug_agent
{
namespace ifdk_objects
{
namespace instance
{

/* Generic reference collection that inherits from a base class
 *
 * In this way it is possible to make a polymorphic list of RefCollection of different types,
 * for instance a list that contains a GenericRefCollection<InstanceRef>, a
 * GenericRefCollection<ServiceRef> and a GenericRefCollection<ComponentRef>.
 */
template <class T>
class GenericRefCollection : public RefCollection
{
private:
    using base = RefCollection;

public:
    GenericRefCollection() = default;
    GenericRefCollection(const std::string &name) : base(name) {}
    explicit GenericRefCollection(const GenericRefCollection &other) = default;
    GenericRefCollection &operator=(const GenericRefCollection &other) = default;

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

    void add(const T &element) { mElements.push_back(element); }

    void resize(std::size_t size) { mElements.resize(size); }

protected:
    virtual bool equalsTo(const RefCollection &other) const noexcept override
    {
        if (!base::equalsTo(other)) {
            return false;
        }

        const GenericRefCollection<T> *otherColl =
            dynamic_cast<const GenericRefCollection<T> *>(&other);
        if (otherColl == nullptr) {
            return false;
        }

        if (mElements.size() != otherColl->mElements.size()) {
            return false;
        }

        for (std::size_t i = 0; i < mElements.size(); ++i) {
            if (mElements[i] != otherColl->mElements[i]) {
                return false;
            }
        }

        return true;
    }

private:
    using ElementVector = std::vector<T>;

    template <typename MyInstance, typename Visitor>
    static void acceptCommon(MyInstance &me, Visitor &visitor)
    {
        visitor.enter(me);

        me.base::accept(visitor, false);

        for (auto &element : me.mElements) {
            element.accept(visitor);
        }

        visitor.leave();
    }

    ElementVector mElements;
};
}
}
}
