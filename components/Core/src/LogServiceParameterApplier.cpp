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
