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
