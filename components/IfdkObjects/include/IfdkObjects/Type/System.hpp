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

#include "IfdkObjects/Type/Component.hpp"
#include <cassert>

namespace debug_agent
{
namespace ifdk_objects
{
namespace type
{

/** Describe a system type.
 */
class System : public Component
{
private:
    using base = Component;
public:
    System() = default;
    explicit System(const std::string& name) : base(name) {}
    explicit System(const System& other) = default;
    System &operator=(const System &other) = default;

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

protected:
    virtual bool equalsTo(const Type &other) const NOEXCEPT override
    {
        if (!base::equalsTo(other)) {
            return false;
        }

        const System *otherSys = dynamic_cast<const System*>(&other);
        if (otherSys == nullptr) {
            return false;
        }

        return true;
    }

private:
    template <typename T, typename Visitor>
    static void acceptCommon(T &me, Visitor &visitor)
    {
        visitor.enter(me);

        me.base::accept(visitor, false);

        visitor.leave();
    }
};
}
}
}


