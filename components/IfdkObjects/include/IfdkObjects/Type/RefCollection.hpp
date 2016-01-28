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

#include "IfdkObjects/Type/Visitor.hpp"
#include <string>

namespace debug_agent
{
namespace ifdk_objects
{
namespace type
{

/** This is the base class of all references collections
 *
 * Each collection has a name.
 */
class RefCollection
{
public:
    RefCollection() = default;
    explicit RefCollection(const std::string &name) : mName(name) {}
    explicit RefCollection(const RefCollection &other) = default;
    virtual ~RefCollection() = default;
    RefCollection &operator=(const RefCollection &other) = default;

    bool operator==(const RefCollection &other) const noexcept
    {
        if (typeid(*this) != typeid(other)) {
            return false;
        }

        return equalsTo(other);
    }

    bool operator!=(const RefCollection &other) const noexcept { return !(*this == other); }

    std::string getName() const { return mName; }

    void setName(const std::string &name) { mName = name; }

    virtual void accept(Visitor &visitor, bool isConcrete = true)
    {
        acceptCommon(*this, visitor, isConcrete);
    }

    virtual void accept(ConstVisitor &visitor, bool isConcrete = true) const
    {
        acceptCommon(*this, visitor, isConcrete);
    }

protected:
    virtual bool equalsTo(const RefCollection &other) const noexcept
    {
        return mName == other.mName;
    }

private:
    template <typename T, typename Visitor>
    static void acceptCommon(T &me, Visitor &visitor, bool isConcrete)
    {
        visitor.enter(me, isConcrete);
        visitor.leave(isConcrete);
    }

    std::string mName;
};
}
}
}
