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
        mSystem.getPerfService().setState(state);
    } catch (PerfService::Exception &e) {
        throw Exception("Unable to set perf measurement parameters: " + std::string(e.what()));
    }
}

std::string PerfServiceParameterApplier::getServiceParameterValue()
{
    Perf::State state;

    try {
        state = mSystem.getPerfService().getState();
    } catch (PerfService::Exception &e) {
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
