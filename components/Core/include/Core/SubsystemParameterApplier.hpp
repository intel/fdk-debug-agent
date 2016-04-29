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
