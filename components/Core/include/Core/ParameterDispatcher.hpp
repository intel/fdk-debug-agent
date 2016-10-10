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
#include "Util/AssertAlways.hpp"
#include <stdexcept>
#include <map>
#include <vector>
#include <set>
#include <memory>
#include <sstream>
#include <string>

namespace debug_agent
{
namespace core
{

/** This class dispatches FDK parameter commands (getStructure, getValue, setValue) to
 * the a ParameterApplier that matches a given type
 */
class ParameterDispatcher
{
public:
    /* Thrown when a command fails. Additional context information is provided. */
    struct Exception : public std::runtime_error
    {
        using std::runtime_error::runtime_error;

        Exception(const std::string &message, const std::string &moduleType, ParameterKind kind,
                  const std::string &moduleInstanceId = std::string(),
                  const std::string &value = std::string())
            : std::runtime_error(formatMessage(message, moduleType, kind, moduleInstanceId, value))
        {
        }

    private:
        static std::string formatMessage(const std::string &message, const std::string &moduleType,
                                         ParameterKind kind, const std::string &moduleInstanceId,
                                         const std::string &value)
        {
            std::stringstream stream;
            stream << "ParameterDispatcher: " << message << " (type=" << moduleType
                   << " kind=" << parameterKindHelper().toString(kind);

            if (moduleInstanceId.length() > 0) {
                stream << " instance=" << moduleInstanceId;
            }

            if (value.length() > 0) {
                stream << "\nvalue:\n" << value << "\n";
            }

            stream << ')';

            return stream.str();
        }
    };

    /** Thrown when a command is not supported */
    class UnsupportedException : public Exception
    {
    public:
        using Exception::Exception;
    };

    ParameterDispatcher(const std::vector<std::shared_ptr<ParameterApplier>> &appliers)
    {
        /* Filling (type, param applier) map*/

        /* Iterating on param appliers */
        for (auto applier : appliers) {
            ASSERT_ALWAYS(applier != nullptr);

            std::set<std::string> supportedTypes = applier->getSupportedTypes();

            /* Iterating on param applier supported types */
            for (auto type : supportedTypes) {

                /* Checking collision */
                auto it = mApplierMap.find(type);
                if (it != mApplierMap.end()) {
                    throw Exception("Duplicated type: " + type);
                }

                /* Adding pair (type, param applier) */
                mApplierMap[type] = applier;
            }
        }
    }

    /** @return the parameter structure of the supplied type */
    std::string getParameterStructure(const std::string &type, ParameterKind kind)
    {
        ParameterApplier &applier = getParamApplierByType(type);
        try {
            return applier.getParameterStructure(type, kind);
        } catch (ParameterApplier::UnsupportedException &) {
            throw UnsupportedException("command not supported: getStructure", type, kind);
        } catch (ParameterApplier::Exception &e) {
            throw Exception("cannot get parameter structure" + std::string(e.what()), type, kind);
        }
    }

    /** @return the parameter value of the supplied type and instance */
    std::string getParameterValue(const std::string &type, ParameterKind kind,
                                  const std::string &instanceId)
    {
        ParameterApplier &applier = getParamApplierByType(type);
        try {
            return applier.getParameterValue(type, kind, instanceId);
        } catch (ParameterApplier::UnsupportedException &) {
            throw UnsupportedException("command not supported: getValue", type, kind, instanceId);
        } catch (ParameterApplier::Exception &e) {
            throw Exception("cannot get parameter value: " + std::string(e.what()), type, kind,
                            instanceId);
        }
    }

    /** Set the parameter value of the supplied type and instance */
    void setParameterValue(const std::string &type, ParameterKind kind,
                           const std::string &instanceId, const std::string &value)
    {
        ParameterApplier &applier = getParamApplierByType(type);
        try {
            applier.setParameterValue(type, kind, instanceId, value);
        } catch (ParameterApplier::UnsupportedException &) {
            throw UnsupportedException("command not supported: setValue", type, kind, instanceId);
        } catch (ParameterApplier::Exception &e) {
            throw Exception("cannot set control parameter value: " + std::string(e.what()), type,
                            kind, instanceId, value);
        }
    }

private:
    using ApplierMap = std::map<std::string, std::shared_ptr<ParameterApplier>>;
    ApplierMap mApplierMap;

    /** Find a ParameterApplier that matches a supplied type */
    ParameterApplier &getParamApplierByType(const std::string &type)
    {
        auto it = mApplierMap.find(type);
        if (it == mApplierMap.end()) {
            throw UnsupportedException("Unsupported type: " + type);
        }
        return *(it->second);
    }
};
}
}
