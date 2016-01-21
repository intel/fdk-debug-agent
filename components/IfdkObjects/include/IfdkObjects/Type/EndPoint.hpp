/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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
