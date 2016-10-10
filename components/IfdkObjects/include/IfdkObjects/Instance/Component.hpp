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
