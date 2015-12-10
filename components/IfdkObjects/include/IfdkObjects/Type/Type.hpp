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

#include "IfdkObjects/Type/Description.hpp"
#include "IfdkObjects/Type/Characteristics.hpp"
#include "IfdkObjects/Type/Children.hpp"
#include "IfdkObjects/Type/InfoParameters.hpp"
#include "IfdkObjects/Type/ControlParameters.hpp"
#include <string>

namespace debug_agent
{
namespace ifdk_objects
{
namespace type
{

/** Base type class
 *
 * This class introduces:
 * - a description
 * - the charachacteristics
 * - info parameters
 * - control parameters
 * - children
 */
class Type
{
public:
    Type() = default;
    explicit Type(const std::string &name) : mName(name) {}
    explicit Type(const Type &other) = default;
    virtual ~Type() = default;
    Type &operator=(const Type &other) = default;

    bool operator==(const Type &other) const NOEXCEPT
    {
        if (typeid(*this) != typeid(other)) {
            return false;
        }

        return equalsTo(other);
    }

    bool operator!=(const Type &other) const NOEXCEPT { return !(*this == other); }

    virtual void accept(Visitor &visitor, bool isConcrete = true)
    {
        acceptCommon(*this, visitor, isConcrete);
    }

    virtual void accept(ConstVisitor &visitor, bool isConcrete = true) const
    {
        acceptCommon(*this, visitor, isConcrete);
    }

    std::string getName() const { return mName; }

    void setName(const std::string &name) { mName = name; }

    Description &getDescription() { return mDescription; }

    Characteristics &getCharacteristics() { return mCharacteristics; }

    Children &getChildren() { return mChildren; }

protected:
    virtual bool equalsTo(const Type &other) const NOEXCEPT
    {
        return mName == other.mName && mDescription == other.mDescription &&
               mCharacteristics == other.mCharacteristics &&
               mInfoParameters == other.mInfoParameters &&
               mControlParameters == other.mControlParameters && mChildren == other.mChildren;
    }

private:
    template <typename T, typename Visitor>
    static void acceptCommon(T &me, Visitor &visitor, bool isConcrete)
    {
        visitor.enter(me, isConcrete);

        me.mDescription.accept(visitor);
        me.mCharacteristics.accept(visitor);
        me.mInfoParameters.accept(visitor);
        me.mControlParameters.accept(visitor);
        me.mChildren.accept(visitor);

        visitor.leave(isConcrete);
    }

    std::string mName;

    Description mDescription;
    Characteristics mCharacteristics;
    InfoParameters mInfoParameters;
    ControlParameters mControlParameters;
    Children mChildren;
};
}
}
}
