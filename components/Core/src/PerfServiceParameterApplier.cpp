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
#include "Core/PerfServiceParameterApplier.hpp"
#include "Core/BaseModelConverter.hpp"

namespace debug_agent
{

using namespace cavs;

namespace core
{

PerfServiceParameterApplier::PerfServiceParameterApplier(cavs::System &system)
    : Base(BaseModelConverter::subsystemName, BaseModelConverter::perfServiceTypeName),
      mSystem(system)
{
}

std::string PerfServiceParameterApplier::getServiceParameterStructure()
{
    /** @todo: currently hardcoded, use libstructure later */
    return R"(<control_parameters>
    <StringParameter Name="State")"
           R"( Description="May be: 'Started', 'Paused', 'Stopped' or 'Disabled'"/>
</control_parameters>
)";
}

void PerfServiceParameterApplier::setServiceParameterValue(const std::string &parameterXML)
{
    static const std::string controlParametersUrl = "/control_parameters/";
    Perf::State state;

    try {
        /** @todo: use libstructure instead of XmlHelper class to fetch values */
        XmlHelper xml(parameterXML);
        state = xml.getValueFromXPathAsEnum<Perf::State>(
            controlParametersUrl + "StringParameter[@Name='State']", Perf::stateHelper());

    } catch (XmlHelper::Exception &e) {
        throw Exception("Cannot parse xml to set control parameter value : " +
                        std::string(e.what()));
    }

    try {
        mSystem.setPerfState(state);
    } catch (System::Exception &e) {
        throw Exception("Unable to set perf measurement parameters: " + std::string(e.what()));
    }
}

std::string PerfServiceParameterApplier::getServiceParameterValue()
{
    Perf::State state;

    try {
        state = mSystem.getPerfState();
    } catch (System::Exception &e) {
        throw Exception(std::string("Cannot get perf measurement parameters : ") + e.what());
    }

    /** @todo: use libstructure to generate xml file */
    std::stringstream stream;
    stream << "<control_parameters>\n"
              "    <StringParameter Name=\"State\">"
           << Perf::stateHelper().toString(state) << "</StringParameter>\n"
           << "</control_parameters>\n";

    return stream.str();
}
}
}
