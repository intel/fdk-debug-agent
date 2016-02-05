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
