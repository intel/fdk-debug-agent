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
