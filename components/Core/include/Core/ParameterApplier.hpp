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

#include "Core/ParameterKind.hpp"
#include <stdexcept>
#include <string>
#include <set>

namespace debug_agent
{
namespace core
{

/** This interface applies FDK parameter commands (getStructure, getValue, setValue) to
 * the subsystem specific part (for instance for cAVS subsystem: log service, modules, ...)
 */
class ParameterApplier
{
public:
    /** Thrown when a command has failed */
    struct Exception : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    /** Thrown  when a command  (getStructure, getValue, setValue) is not supported */
    struct UnsupportedException : public Exception
    {
        /* No message required */
        UnsupportedException() : Exception("") {}
    };

    ParameterApplier() = default;
    virtual ~ParameterApplier() = default;
    ParameterApplier(const ParameterApplier &) = delete;
    ParameterApplier &operator=(const ParameterApplier &) = delete;

    virtual std::set<std::string> getSupportedTypes() const = 0;

    /** @return the parameter structure of the supplied type */
    virtual std::string getParameterStructure(const std::string &type, ParameterKind kind) = 0;

    /** @return the parameter value of the supplied type and instance */
    virtual std::string getParameterValue(const std::string &type, ParameterKind kind,
                                          const std::string &instanceId) = 0;

    /** Set the parameter value of the supplied type and instance */
    virtual void setParameterValue(const std::string &type, ParameterKind kind,
                                   const std::string &instanceId,
                                   const std::string &parameterValue) = 0;
};
}
}
