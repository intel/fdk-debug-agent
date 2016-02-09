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
