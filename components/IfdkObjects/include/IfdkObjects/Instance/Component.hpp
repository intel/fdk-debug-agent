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

#include "IfdkObjects/Instance/Instance.hpp"
#include "IfdkObjects/Instance/Inputs.hpp"
#include "IfdkObjects/Instance/Outputs.hpp"
#include "IfdkObjects/Instance/Links.hpp"
#include <vector>
#include <memory>

namespace debug_agent
{
namespace ifdk_objects
{
namespace instance
{

/** The Component class introduces
 * - inputs and output connectors
 * - the links
 */
class Component : public Instance
{
private:
    using base = Instance;

public:
    Component() = default;
    explicit Component(const std::string &typeName, const std::string &instanceId)
        : base(typeName, instanceId)
    {
    }
    explicit Component(const Component &other) = default;
    Component &operator=(const Component &other) = default;

    virtual void accept(Visitor &visitor, bool isConcrete = true) override
    {
        acceptCommon(*this, visitor, isConcrete);
    }

    virtual void accept(ConstVisitor &visitor, bool isConcrete = true) const override
    {
        acceptCommon(*this, visitor, isConcrete);
    }

    Inputs &getInputs() { return mInputs; }

    Outputs &getOutputs() { return mOutputs; }

    Links &getLinks() { return mLinks; }

protected:
    virtual bool equalsTo(const Instance &other) const noexcept override
    {
        if (!base::equalsTo(other)) {
            return false;
        }

        const Component *otherComp = dynamic_cast<const Component *>(&other);
        if (otherComp == nullptr) {
            return false;
        }

        return mInputs == otherComp->mInputs && mOutputs == otherComp->mOutputs &&
               mLinks == otherComp->mLinks;
    }

private:
    template <typename T, typename Visitor>
    static void acceptCommon(T &me, Visitor &visitor, bool isConcrete)
    {
        visitor.enter(me, isConcrete);

        me.base::accept(visitor, false);

        me.mInputs.accept(visitor);
        me.mOutputs.accept(visitor);
        me.mLinks.accept(visitor);

        visitor.leave(isConcrete);
    }

    Inputs mInputs;
    Outputs mOutputs;
    Links mLinks;
};
}
}
}
