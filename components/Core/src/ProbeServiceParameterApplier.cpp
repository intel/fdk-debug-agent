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
        mSystem.getProbeService().setState(active);
    } catch (ProbeService::Exception &e) {
        throw Exception("Unable to set probe service state: " + std::string(e.what()));
    }
}

std::string ProbeServiceParameterApplier::getServiceParameterValue()
{
    bool active;

    try {
        active = mSystem.getProbeService().isActive();
    } catch (ProbeService::Exception &e) {
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
