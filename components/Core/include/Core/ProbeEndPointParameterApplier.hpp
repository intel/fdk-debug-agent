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

#include "Core/EndPointParameterApplier.hpp"
#include "cAVS/System.hpp"

namespace debug_agent
{
namespace core
{

class ProbeEndPointParameterApplier : public EndPointParameterApplier
{
public:
    ProbeEndPointParameterApplier(cavs::System &system);

    std::string getEndPointParameterStructure() override;

    std::string getEndPointParameterValue(std::size_t endPointIndex) override;

    void setEndPointParameterValue(std::size_t endPointIndex,
                                   const std::string &parameterValue) override;

private:
    static std::string getCavsModuleNameFromFdk(const std::string &fdkModuleType);
    static std::string getFdkModuleNameFromCavs(const std::string &cavsModuleType);

    /** Set unused Prober::ProbeConfig structure members to a valid value for lowest layers
     * when the "enabled" member is false */
    static void setUnusedMembers(cavs::Prober::ProbeConfig &config);

    using Base = EndPointParameterApplier;
    cavs::System &mSystem;
};
}
}
