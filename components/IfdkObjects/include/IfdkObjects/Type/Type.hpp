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

    bool operator==(const Type &other) const noexcept
    {
        if (typeid(*this) != typeid(other)) {
            return false;
        }

        return equalsTo(other);
    }

    bool operator!=(const Type &other) const noexcept { return !(*this == other); }

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
    virtual bool equalsTo(const Type &other) const noexcept
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
