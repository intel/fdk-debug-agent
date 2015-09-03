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
#include <memory>
#include <assert.h>

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
    explicit VisitablePtrVector(const VisitablePtrVector& other) = default;
    VisitablePtrVector &operator=(const VisitablePtrVector &other) = default;

    /** The == operator tests pointed value, not the pointer value */
    bool operator == (const VisitablePtrVector &other) const NOEXCEPT
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

    bool operator != (const VisitablePtrVector &other) const NOEXCEPT
    {
        return !(*this == other);
    }

    /* A visitable element should implement the accept() methods (const and non const) */

    void accept(Visitor &visitor)
    {
        acceptCommon(*this, visitor);
    }

    void accept(ConstVisitor &visitor) const
    {
        acceptCommon(*this, visitor);
    }

    void add(ElementType *element)
    {
        assert(element != nullptr);
        mElementPtrs.push_back(std::shared_ptr<ElementType>(element));
    }

    void add(std::shared_ptr<ElementType> element)
    {
        assert(element != nullptr);
        mElementPtrs.push_back(element);
    }

    ElementPtrVector &getElements()
    {
        return mElementPtrs;
    }

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



