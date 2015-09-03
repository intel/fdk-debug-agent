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

#include "IfdkObjects/Type/RefCollection.hpp"
#include <vector>
#include <assert.h>

namespace debug_agent
{
namespace ifdk_objects
{
namespace type
{

/* Generic reference collection that inherits from a base class
 *
 * In this way it is possible to make a polymorphic list of RefCollection of different types,
 * for instance a list that contains a GenericRefCollection<TypeRef>, a
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
    explicit GenericRefCollection(const GenericRefCollection& other) = default;
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

    void add(const T &element)
    {
        mElements.push_back(element);
    }

    void resize(std::size_t size)
    {
        mElements.resize(size);
    }

protected:
    virtual bool equalsTo(const RefCollection &other) const NOEXCEPT override
    {
        if (!base::equalsTo(other)) {
            return false;
        }

        const GenericRefCollection<T> *otherColl =
            dynamic_cast<const GenericRefCollection<T>*>(&other);
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

    template <typename MyType, typename Visitor>
    static void acceptCommon(MyType &me, Visitor &visitor)
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


