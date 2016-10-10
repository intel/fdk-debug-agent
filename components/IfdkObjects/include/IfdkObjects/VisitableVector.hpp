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
#include <cassert>

namespace debug_agent
{
namespace ifdk_objects
{

/** Implements a generic visitable vector
 *
 * @tparam ElementType class of elements
 * @tparam Visitor The used visitor class
 * @tparam ConstVisitor The used constant visitor class
 */
template <class ElementType, class Visitor, class ConstVisitor>
class VisitableVector final
{
public:
    using ElementVector = std::vector<ElementType>;

    VisitableVector() = default;
    explicit VisitableVector(const VisitableVector &other) = default;
    VisitableVector &operator=(const VisitableVector &other) = default;

    bool operator==(const VisitableVector &other) const noexcept
    {
        if (mElements.size() != other.mElements.size()) {
            return false;
        }

        for (std::size_t i = 0; i < mElements.size(); ++i) {
            if (mElements[i] != other.mElements[i]) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const VisitableVector &other) const noexcept { return !(*this == other); }

    /* A visitable element should implement the accept() methods (const and non const) */

    void accept(Visitor &visitor) { acceptCommon(*this, visitor); }

    void accept(ConstVisitor &visitor) const { acceptCommon(*this, visitor); }

    void add(const ElementType &element) { mElements.push_back(element); }

    void resize(std::size_t size) { mElements.resize(size); }

    ElementVector &getElements() { return mElements; }

private:
    /* This template trick is used to factorize the accept() method for both const and non-const
     * versions */
    template <typename T, typename VisitorType>
    static void acceptCommon(T &me, VisitorType &visitor)
    {
        visitor.enter(me);

        for (auto &type : me.mElements) {
            type.accept(visitor);
        }

        visitor.leave();
    }

    ElementVector mElements;
};
}
}
