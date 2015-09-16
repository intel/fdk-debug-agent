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

#include "IfdkObjects/Instance/BaseCollection.hpp"
#include <vector>
#include <memory>
#include <assert.h>

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
    explicit GenericCollection(const GenericCollection& other) = default;
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

    virtual std::shared_ptr<const Instance> getInstance(const std::string &instanceId) const override
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

    void add(T *element)
    {
        add(std::shared_ptr<T>(element));
    }

    void add(std::shared_ptr<T> element)
    {
        assert(element != nullptr);
        mElementPtrs.push_back(element);
    }

protected:
    virtual bool equalsTo(const BaseCollection &other) const NOEXCEPT override
    {
        const GenericCollection<T> *otherColl =
            dynamic_cast<const GenericCollection<T>*>(&other);
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
    using ElementPtrVector= std::vector<std::shared_ptr<T>>;

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


