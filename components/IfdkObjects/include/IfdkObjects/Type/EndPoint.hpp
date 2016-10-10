/*
 * Copyright (c) 2016, Intel Corporation
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

#include "IfdkObjects/Type/Type.hpp"
#include "Util/EnumHelper.hpp"

namespace debug_agent
{
namespace ifdk_objects
{
namespace type
{

/** Type of service */
class EndPoint final : public Type
{
private:
    using base = Type;

public:
    /** Endpoint direction */
    enum class Direction
    {
        Unknown,
        Outgoing, /* Outgoing from the service point of view */
        Incoming, /* Incoming from the service point of view */
        Bidirectional
    };

    static const util::EnumHelper<Direction> &directionHelper()
    {
        static util::EnumHelper<Direction> helper({
            {Direction::Unknown, "Unknown"},
            {Direction::Outgoing, "Outgoing"},
            {Direction::Incoming, "Incoming"},
            {Direction::Bidirectional, "Bidirectional"},
        });
        return helper;
    }

    EndPoint() = default;
    explicit EndPoint(const std::string &name, Direction direction)
        : base(name), mDirection(direction)
    {
    }
    using base::base;
    using base::operator=;

    void accept(Visitor &visitor, bool isConcrete = true) override
    {
        assert(isConcrete);
        acceptCommon(*this, visitor);
    }

    void accept(ConstVisitor &visitor, bool isConcrete = true) const override
    {
        assert(isConcrete);
        acceptCommon(*this, visitor);
    }

    Direction getDirection() const { return mDirection; }

    void setDirection(Direction dir) { mDirection = dir; }

protected:
    bool equalsTo(const Type &other) const noexcept override
    {
        if (!base::equalsTo(other)) {
            return false;
        }

        const EndPoint *otherEp = dynamic_cast<const EndPoint *>(&other);
        if (otherEp == nullptr) {
            return false;
        }

        return mDirection == otherEp->mDirection;
    }

private:
    template <typename T, typename Visitor>
    static void acceptCommon(T &me, Visitor &visitor)
    {
        visitor.enter(me);

        me.base::accept(visitor, false);

        visitor.leave(true);
    }

    Direction mDirection = Direction::Unknown;
};
}
}
}
