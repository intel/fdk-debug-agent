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

#include "XmlHelper.hpp"
#include "Core/ProbeServiceParameterApplier.hpp"
#include "Core/BaseModelConverter.hpp"

namespace debug_agent
{

using namespace cavs;

namespace core
{

ProbeServiceParameterApplier::ProbeServiceParameterApplier(cavs::System &system)
    : Base(BaseModelConverter::subsystemName, BaseModelConverter::probeServiceTypeName),
      mSystem(system)
{
}

std::string ProbeServiceParameterApplier::getServiceParameterStructure()
{
    /** @todo: currently hardcoded, use libstructure later */
    return "<control_parameters>\n"
           "    <ParameterBlock Name=\"State\">\n"
           "        <BooleanParameter Name=\"Started\"/>\n"
           "    </ParameterBlock>\n"
           "</control_parameters>\n";
}

void ProbeServiceParameterApplier::setServiceParameterValue(const std::string &parameterXML)
{
    static const std::string stateBlockUrl = "/control_parameters/ParameterBlock[@Name='State']/";
    bool active;

    try {
        /** @todo: use libstructure instead of XmlHelper class to fetch values */
        XmlHelper xml(parameterXML);
        active = xml.getValueFromXPath<bool>(stateBlockUrl + "BooleanParameter[@Name='Started']");

    } catch (XmlHelper::Exception &e) {
        throw Exception("Cannot parse xml to set probe service parameter value : " +
                        std::string(e.what()));
    }

    try {
        mSystem.setProberState(active);
    } catch (System::Exception &e) {
        throw Exception("Unable to set probe service state: " + std::string(e.what()));
    }
}

std::string ProbeServiceParameterApplier::getServiceParameterValue()
{
    bool active;

    try {
        active = mSystem.isProberActive();
    } catch (System::Exception &e) {
        throw Exception(std::string("Cannot get probe service state: ") + e.what());
    }

    /** @todo: use libstructure to generate xml file */
    std::stringstream stream;
    stream << "<control_parameters>\n"
              "    <ParameterBlock Name=\"State\">\n"
              "        <BooleanParameter Name=\"Started\">"
           << (active ? "1" : "0") << "</BooleanParameter>\n"
                                      "    </ParameterBlock>\n"
                                      "</control_parameters>\n";

    return stream.str();
}
}
}
