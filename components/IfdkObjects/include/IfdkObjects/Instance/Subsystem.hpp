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

#include "IfdkObjects/Instance/Component.hpp"
#include <vector>
#include <memory>
#include <assert.h>

namespace debug_agent
{
namespace ifdk_objects
{
namespace instance
{

/** Describe a subsystem instance.
 *
 * This class introduces categories.
 */
class Subsystem : public Component
{
private:
    using base = Component;
public:
    Subsystem() = default;
    explicit Subsystem(const std::string& typeName, const std::string& instanceId) :
        base(typeName, instanceId) {}
    explicit Subsystem(const Subsystem& other) = default;
    Subsystem &operator=(const Subsystem &other) = default;

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
    virtual bool equalsTo(const Instance &other) const NOEXCEPT override
    {
        if (!base::equalsTo(other)) {
            return false;
        }

        const Subsystem *otherComp = dynamic_cast<const Subsystem*>(&other);
        if (otherComp == nullptr) {
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


