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
#include "Core/ProbeEndPointParameterApplier.hpp"
#include "Core/BaseModelConverter.hpp"
#include "Util/AssertAlways.hpp"

namespace debug_agent
{

using namespace cavs;

namespace core
{

void ProbeEndPointParameterApplier::setUnusedMembers(Prober::ProbeConfig &config)
{
    ASSERT_ALWAYS(!config.enabled);

    /* Config is disabled.
     * In this case other members should not be used. So initializing them to arbitrary
     * values that will not make fail lowest layers
     */
    config.purpose = Prober::ProbePurpose::Inject;
    config.probePoint.fields.setModuleId(0);
    config.probePoint.fields.setInstanceId(0);
    config.probePoint.fields.setType(dsp_fw::ProbeType::Input);
    config.probePoint.fields.setIndex(0);
}

ProbeEndPointParameterApplier::ProbeEndPointParameterApplier(System &system)
    : Base(BaseModelConverter::subsystemName, BaseModelConverter::probeServiceTypeName,
           BaseModelConverter::probeServiceEndPointCount),
      mSystem(system)
{
}

std::string ProbeEndPointParameterApplier::getEndPointParameterStructure()
{
    /** @todo: currently hardcoded, use libstructure later */
    return R"(<control_parameters>
    <ParameterBlock Name="ProbePointConnection">
        <BooleanParameter Name="Active"/>
        <EnumParameter Name="Purpose">
            <ValuePair Literal="Extract" Numerical="0"/>
            <ValuePair Literal="Inject" Numerical="1"/>
            <ValuePair Literal="InjectReextract" Numerical="2"/>
        </EnumParameter>
        <StringParameter Name="ComponentType" Size="100"/>
        <IntegerParameter Name="InstanceId" Size="16"/>
        <EnumParameter Name="Type">
            <ValuePair Literal="Input" Numerical="0"/>
            <ValuePair Literal="Output" Numerical="1"/>
            <ValuePair Literal="Internal" Numerical="2"/>
        </EnumParameter>
        <IntegerParameter Name="Index" Size="16"/>
    </ParameterBlock>
</control_parameters>)";
}

void ProbeEndPointParameterApplier::setEndPointParameterValue(const std::size_t endPointIndex,
                                                              const std::string &parameterXML)
{
    cavs::ProbeId probeId(static_cast<uint32_t>(endPointIndex));

    Prober::ProbeConfig config;

    static const std::string connectionBlockUrl =
        "/control_parameters/ParameterBlock[@Name='ProbePointConnection']/";

    try {
        /** @todo: use libstructure instead of XmlHelper class to fetch values */
        XmlHelper xml(parameterXML);

        config.enabled =
            xml.getValueFromXPath<bool>(connectionBlockUrl + "BooleanParameter[@Name='Active']");

        /* Fetching values only if probe is enabled*/
        if (config.enabled) {
            config.purpose = xml.getValueFromXPathAsEnum<Prober::ProbePurpose>(
                connectionBlockUrl + "EnumParameter[@Name='Purpose']",
                Prober::probePurposeHelper());

            std::string fdkModuleTypeName = xml.getValueFromXPathAsString(
                connectionBlockUrl + "StringParameter[@Name='ComponentType']");

            std::string cavsModuleTypeName = getCavsModuleNameFromFdk(fdkModuleTypeName);
            try {
                config.probePoint.fields.setModuleId(
                    mSystem.getModuleHandler().findModuleEntry(cavsModuleTypeName).module_id);
            } catch (System::Exception &e) {
                throw Exception("Cannot get cavs module id: " + std::string(e.what()));
            }

            config.probePoint.fields.setInstanceId(xml.getValueFromXPath<uint32_t>(
                connectionBlockUrl + "IntegerParameter[@Name='InstanceId']"));
            config.probePoint.fields.setType(xml.getValueFromXPathAsEnum<dsp_fw::ProbeType>(
                connectionBlockUrl + "EnumParameter[@Name='Type']", dsp_fw::probeTypeHelper()));
            config.probePoint.fields.setIndex(xml.getValueFromXPath<uint32_t>(
                connectionBlockUrl + "IntegerParameter[@Name='Index']"));
        } else {
            setUnusedMembers(config);
        }

    } catch (XmlHelper::Exception &e) {
        throw Exception("Cannot parse xml to set probe endpoint parameter value : " +
                        std::string(e.what()));
    } catch (dsp_fw::ProbePointId::Exception &e) {
        throw Exception("Cannot set probe point id: " + std::string(e.what()));
    }

    /* Setting probe configuration to the driver */
    try {
        mSystem.getProbeService().setConfiguration(probeId, config);
    } catch (ProbeService::Exception &e) {
        throw Exception("Unable to set probe configuration: " + std::string(e.what()));
    }
}

std::string ProbeEndPointParameterApplier::getEndPointParameterValue(
    const std::size_t endPointIndex)
{
    cavs::ProbeId probeId(static_cast<uint32_t>(endPointIndex));

    Prober::ProbeConfig config;
    try {
        config = mSystem.getProbeService().getConfiguration(probeId);
    } catch (ProbeService::Exception &e) {
        throw Exception("Unable to get probe configuration: " + std::string(e.what()));
    }

    /* Getting fdk module type name from cAVS module type id */
    std::string fdkModuleTypeName;
    if (config.enabled) {
        try {
            std::string cavsModuleTypeName =
                mSystem.getModuleHandler()
                    .findModuleEntry(config.probePoint.fields.getModuleId())
                    .getName();
            fdkModuleTypeName = getFdkModuleNameFromCavs(cavsModuleTypeName);
        } catch (System::Exception &e) {
            throw Exception("Cannot get cavs module name: " + std::string(e.what()));
        }
    } else {
        /* Endpoint is disabled, settings unused members to an arbitrary value that will
         * not make fail the parsing of the produced xml */
        fdkModuleTypeName = "_Invalid_";
        setUnusedMembers(config);
    }

    /** @todo: use libstructure to generate xml file */
    std::stringstream stream;
    stream << "<control_parameters>\n"
           << "    <ParameterBlock Name=\"ProbePointConnection\">\n"
           << "        <BooleanParameter Name=\"Active\">" << (config.enabled ? "1" : "0")
           << "</BooleanParameter>\n"
           << "        <EnumParameter Name=\"Purpose\">"
           << Prober::probePurposeHelper().toString(config.purpose) << "</EnumParameter>\n"
           << "        <StringParameter Name=\"ComponentType\">" << fdkModuleTypeName
           << "</StringParameter>\n"
           << "        <IntegerParameter Name=\"InstanceId\">"
           << config.probePoint.fields.getInstanceId() << "</IntegerParameter>\n"
           << "        <EnumParameter Name=\"Type\">"
           << dsp_fw::probeTypeHelper().toString(config.probePoint.fields.getType())
           << "</EnumParameter>\n"
           << "        <IntegerParameter Name=\"Index\">" << config.probePoint.fields.getIndex()
           << "</IntegerParameter>\n"
           << "    </ParameterBlock>\n"
           << "</control_parameters>\n";
    return stream.str();
}

std::string ProbeEndPointParameterApplier::getCavsModuleNameFromFdk(
    const std::string &fdkModuleType)
{
    if (!util::StringHelper::startWith(fdkModuleType, BaseModelConverter::modulePrefix)) {
        throw Exception("Wrong fdk module type: " + fdkModuleType);
    }

    // removing prefix
    return fdkModuleType.substr(BaseModelConverter::modulePrefix.length(),
                                fdkModuleType.length() - BaseModelConverter::modulePrefix.length());
}

std::string ProbeEndPointParameterApplier::getFdkModuleNameFromCavs(
    const std::string &cavsModuleType)
{
    return BaseModelConverter::modulePrefix + cavsModuleType;
}
}
}
