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

#include <vector>
#include <memory>
#include <cassert>

namespace debug_agent
{
namespace ifdk_objects
{

/** Generic visitable vector of shared ptr.
 *
 * This vector is suitable for polymorphic lists.
 *
 * @tparam ElementType base class of elements
 * @tparam Visitor The used visitor class
 * @tparam ConstVisitor The used constant visitor class
 */
template <class ElementType, class Visitor, class ConstVisitor>
class VisitablePtrVector final
{
public:
    using ElementPtrVector = std::vector<std::shared_ptr<ElementType>>;

    VisitablePtrVector() = default;
    explicit VisitablePtrVector(const VisitablePtrVector &other) = default;
    VisitablePtrVector &operator=(const VisitablePtrVector &other) = default;

    /** The == operator tests pointed value, not the pointer value */
    bool operator==(const VisitablePtrVector &other) const noexcept
    {
        if (mElementPtrs.size() != other.mElementPtrs.size()) {
            return false;
        }

        for (std::size_t i = 0; i < mElementPtrs.size(); ++i) {
            /* Testing pointer content */
            if (*mElementPtrs[i] != *other.mElementPtrs[i]) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const VisitablePtrVector &other) const noexcept { return !(*this == other); }

    /* A visitable element should implement the accept() methods (const and non const) */

    void accept(Visitor &visitor) { acceptCommon(*this, visitor); }

    void accept(ConstVisitor &visitor) const { acceptCommon(*this, visitor); }

    void add(std::shared_ptr<ElementType> element)
    {
        assert(element != nullptr);
        mElementPtrs.push_back(element);
    }

    ElementPtrVector &getElements() { return mElementPtrs; }

private:
    /* This template trick is used to factorize the accept() method for both const and non-const
     * versions */
    template <typename T, typename VisitorType>
    static void acceptCommon(T &me, VisitorType &visitor)
    {
        visitor.enter(me);

        for (auto type : me.mElementPtrs) {
            type->accept(visitor);
        }

        visitor.leave();
    }

    ElementPtrVector mElementPtrs;
};
}
}
