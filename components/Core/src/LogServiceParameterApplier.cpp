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
#include "Core/LogServiceParameterApplier.hpp"
#include "Core/BaseModelConverter.hpp"
#include "cAVS/Logger.hpp"
#include <type_traits>

namespace debug_agent
{

using namespace cavs;

namespace core
{

LogServiceParameterApplier::LogServiceParameterApplier(cavs::System &system)
    : Base(BaseModelConverter::subsystemName, BaseModelConverter::logServiceTypeName),
      mSystem(system)
{
}

std::string LogServiceParameterApplier::getServiceParameterStructure()
{
    /** @todo: currently hardcoded, use libstructure later */
    auto toString = [](auto level) {
        return std::to_string(static_cast<std::underlying_type_t<decltype(level)>>(level));
    };

    auto strValuePair = [&](auto numerical, auto literal) {
        return "<ValuePair Numerical=\"" + toString(numerical) + "\" Literal=\"" + literal + "\"/>";
    };

    using Level = cavs::Logger::Level;

    return R"(<control_parameters>
    <BooleanParameter Name="Started"/>
    <ParameterBlock Name="Buffering">
        <IntegerParameter Name="Size" Size="16" Unit="MegaBytes"/>
        <BooleanParameter Name="Circular"/>
    </ParameterBlock>
    <BooleanParameter Name="PersistsState"/>
    <EnumParameter Size="8" Name="Verbosity">
        )" +
           strValuePair(Level::Critical, "Critical") + R"(
        )" +
           strValuePair(Level::High, "High") + R"(
        )" +
           strValuePair(Level::Medium, "Medium") + R"(
        )" +
           strValuePair(Level::Low, "Low") + R"(
        )" +
           strValuePair(Level::Verbose, "Verbose") + R"(
    </EnumParameter>
    <BooleanParameter Name="ViaPTI" Description="Set to 1 if PTI interface is to be used"/>
</control_parameters>
)";
}

void LogServiceParameterApplier::setServiceParameterValue(const std::string &parameterXML)
{
    static const std::string controlParametersUrl = "/control_parameters/";
    Logger::Parameters logParameters;

    try {
        /** @todo: use libstructure instead of XmlHelper class to fetch values */
        XmlHelper xml(parameterXML);
        logParameters.mIsStarted =
            xml.getValueFromXPath<bool>(controlParametersUrl + "BooleanParameter[@Name='Started']");
        logParameters.mLevel = xml.getValueFromXPathAsEnum<Logger::Level>(
            controlParametersUrl + "EnumParameter[@Name='Verbosity']", Logger::levelHelper());
        logParameters.mOutput =
            xml.getValueFromXPath<bool>(controlParametersUrl + "BooleanParameter[@Name='ViaPTI']")
                ? Logger::Output::Pti
                : Logger::Output::Sram;

    } catch (XmlHelper::Exception &e) {
        throw Exception("Cannot parse xml to set control parameter value : " +
                        std::string(e.what()));
    }

    try {
        mSystem.setLogParameters(logParameters);
    } catch (System::Exception &e) {
        throw Exception("Unable to set log parameters: " + std::string(e.what()));
    }
}

std::string LogServiceParameterApplier::getServiceParameterValue()
{
    Logger::Parameters logParameters;

    try {
        logParameters = mSystem.getLogParameters();
    } catch (System::Exception &e) {
        throw Exception(std::string("Cannot get log parameters : ") + e.what());
    }

    /** @todo: use libstructure to generate xml file */
    std::stringstream stream;
    stream << "<control_parameters>\n"
              "    <BooleanParameter Name=\"Started\">"
           << logParameters.mIsStarted
           << "</BooleanParameter>\n"
              "    <ParameterBlock Name=\"Buffering\">\n"
              "        <IntegerParameter Name=\"Size\">100</IntegerParameter>\n"
              "        <BooleanParameter Name=\"Circular\">0</BooleanParameter>\n"
              "    </ParameterBlock>\n"
              "    <BooleanParameter Name=\"PersistsState\">0</BooleanParameter>\n"
              "    <EnumParameter Name=\"Verbosity\">"
           << Logger::levelHelper().toString(logParameters.mLevel)
           << "</EnumParameter>\n"
              "    <BooleanParameter Name=\"ViaPTI\">"
           << (logParameters.mOutput == Logger::Output::Pti ? 1 : 0) << "</BooleanParameter>\n"
                                                                        "</control_parameters>\n";

    return stream.str();
}
}
}
