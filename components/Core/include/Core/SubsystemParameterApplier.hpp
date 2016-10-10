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

#include "Core/ParameterApplier.hpp"
#include "cAVS/System.hpp"
#include "Util/Exception.hpp"
#include <string>
#include <set>

namespace debug_agent
{
namespace core
{

class SubsystemParameterApplier : public ParameterApplier
{
public:
    using Exception = util::Exception<SubsystemParameterApplier>;

    SubsystemParameterApplier(cavs::System &system) : mSystem(system) {}

    std::set<std::string> getSupportedTypes() const override;

    std::string getParameterStructure(const std::string & /*type*/, ParameterKind /*kind*/) override
    {
        throw UnsupportedException();
    }

    std::string getParameterValue(const std::string &type, ParameterKind kind,
                                  const std::string &instanceId) override;

    void setParameterValue(const std::string & /*type*/, ParameterKind /*kind*/,
                           const std::string & /*instanceId*/,
                           const std::string & /*parameterValue*/) override
    {
        throw UnsupportedException();
    }

private:
    cavs::System &mSystem;
};
}
}
