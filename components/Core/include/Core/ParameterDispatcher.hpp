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
            throw Exception("cannot get parameter value" + std::string(e.what()), type, kind,
                            instanceId);
        }
    }

    /** Set the parameter value of the supplied type and instance */
    void setParameterValue(const std::string &type, ParameterKind kind,
                           const std::string &instanceId, const std::string &value)
    {
        ParameterApplier &applier = getParamApplierByType(type);
        try {
            return applier.setParameterValue(type, kind, instanceId, value);
        } catch (ParameterApplier::UnsupportedException &) {
            throw UnsupportedException("command not supported: setValue", type, kind, instanceId);
        } catch (ParameterApplier::Exception &e) {
            throw Exception("cannot set control parameter value" + std::string(e.what()), type,
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
