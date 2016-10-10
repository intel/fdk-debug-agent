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
#include "Util/convert.hpp"

namespace debug_agent
{
namespace core
{

/** Base ParameterApplier class for all endpoints.
 *
 * Currently end points support only control parameters.
 */
class EndPointParameterApplier : public ParameterApplier
{
public:
    EndPointParameterApplier(const std::string &subsystemName, const std::string &serviceName,
                             std::size_t endPointCount)
        : mEndPointType(subsystemName + "." + serviceName + ".endpoint"),
          mEndPointCount(endPointCount)
    {
    }

    std::set<std::string> getSupportedTypes() const override final { return {mEndPointType}; }

    std::string getParameterStructure(const std::string & /*type*/,
                                      ParameterKind kind) override final
    {
        checkKind(kind);
        return getEndPointParameterStructure();
    }

    std::string getParameterValue(const std::string & /*type*/, ParameterKind kind,
                                  const std::string &instanceId) override final
    {
        checkKind(kind);
        std::size_t endPointIndex = checkInstanceId(instanceId);
        return getEndPointParameterValue(endPointIndex);
    }

    void setParameterValue(const std::string & /*type*/, ParameterKind kind,
                           const std::string &instanceId,
                           const std::string &parameterValue) override final
    {
        checkKind(kind);
        std::size_t endPointIndex = checkInstanceId(instanceId);
        setEndPointParameterValue(endPointIndex, parameterValue);
    }

private:
    virtual std::string getEndPointParameterStructure() = 0;
    virtual std::string getEndPointParameterValue(std::size_t endPointIndex) = 0;
    virtual void setEndPointParameterValue(std::size_t endPointIndex,
                                           const std::string &parameterValue) = 0;

    void checkKind(ParameterKind kind)
    {
        if (kind != ParameterKind::Control) {
            throw UnsupportedException();
        }
    }

    std::size_t checkInstanceId(const std::string &instanceId)
    {
        std::size_t numId;
        if (!::convertTo(instanceId, numId) || numId >= mEndPointCount) {
            throw UnsupportedException();
        }
        return numId;
    }

    const std::string mEndPointType;
    const std::size_t mEndPointCount;
};
}
}
