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

namespace debug_agent
{
namespace core
{

/** Base ParameterApplier class for all services.
 *
 * Currenly services don't support info parameters, and have only one instance "0"
 */
class ServiceParameterApplier : public ParameterApplier
{
public:
    ServiceParameterApplier(const std::string &subsystemName, const std::string &serviceName)
        : mServiceType(subsystemName + "." + serviceName), mInstanceId("0")
    {
    }

    std::set<std::string> getSupportedTypes() const override final { return {mServiceType}; }

    std::string getParameterStructure(const std::string & /*type*/,
                                      ParameterKind kind) override final
    {
        checkKind(kind);
        return getServiceParameterStructure();
    }

    std::string getParameterValue(const std::string & /*type*/, ParameterKind kind,
                                  const std::string &instanceId) override final
    {
        checkKind(kind);
        checkInstanceId(instanceId);
        return getServiceParameterValue();
    }

    void setParameterValue(const std::string & /*type*/, ParameterKind kind,
                           const std::string &instanceId,
                           const std::string &parameterValue) override final
    {
        checkKind(kind);
        checkInstanceId(instanceId);
        setServiceParameterValue(parameterValue);
    }

private:
    virtual std::string getServiceParameterStructure() = 0;
    virtual std::string getServiceParameterValue() = 0;
    virtual void setServiceParameterValue(const std::string &parameterValue) = 0;

    void checkKind(ParameterKind kind)
    {
        if (kind != ParameterKind::Control) {
            throw UnsupportedException();
        }
    }

    void checkInstanceId(const std::string &instanceId)
    {
        if (instanceId != mInstanceId) {
            throw UnsupportedException();
        }
    }

    const std::string mServiceType;
    const std::string mInstanceId;
};
}
}
