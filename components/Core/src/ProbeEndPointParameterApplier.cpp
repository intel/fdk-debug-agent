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
    config.probePoint.moduleTypeId = 0;
    config.probePoint.moduleInstanceId = 0;
    config.probePoint.type = Prober::ProbeType::Input;
    config.probePoint.pinIndex = 0;
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
                config.probePoint.moduleTypeId =
                    mSystem.findModuleEntry(cavsModuleTypeName).module_id;
            } catch (System::Exception &e) {
                throw Exception("Cannot get cavs module id: " + std::string(e.what()));
            }

            config.probePoint.moduleInstanceId =
                xml.getValueFromXPath<decltype(config.probePoint.moduleInstanceId)>(
                    connectionBlockUrl + "IntegerParameter[@Name='InstanceId']");
            config.probePoint.type = xml.getValueFromXPathAsEnum<Prober::ProbeType>(
                connectionBlockUrl + "EnumParameter[@Name='Type']", Prober::probeTypeHelper());
            config.probePoint.pinIndex =
                xml.getValueFromXPath<decltype(config.probePoint.pinIndex)>(
                    connectionBlockUrl + "IntegerParameter[@Name='Index']");
        } else {
            setUnusedMembers(config);
        }

    } catch (XmlHelper::Exception &e) {
        throw Exception("Cannot parse xml to set probe endpoint parameter value : " +
                        std::string(e.what()));
    }

    /* Setting probe configuration to the driver */
    try {
        mSystem.setProbeConfiguration(probeId, config);
    } catch (System::Exception &e) {
        throw Exception("Unable to set probe configuration: " + std::string(e.what()));
    }
}

std::string ProbeEndPointParameterApplier::getEndPointParameterValue(
    const std::size_t endPointIndex)
{
    cavs::ProbeId probeId(static_cast<uint32_t>(endPointIndex));

    Prober::ProbeConfig config;
    try {
        config = mSystem.getProbeConfiguration(probeId);
    } catch (System::Exception &e) {
        throw Exception("Unable to get probe configuration: " + std::string(e.what()));
    }

    /* Getting fdk module type name from cAVS module type id */
    std::string fdkModuleTypeName;
    if (config.enabled) {
        try {
            std::string cavsModuleTypeName =
                mSystem.findModuleEntry(config.probePoint.moduleTypeId).getName();
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
           << "        <IntegerParameter Name=\"InstanceId\">" << config.probePoint.moduleInstanceId
           << "</IntegerParameter>\n"
           << "        <EnumParameter Name=\"Type\">"
           << Prober::probeTypeHelper().toString(config.probePoint.type) << "</EnumParameter>\n"
           << "        <IntegerParameter Name=\"Index\">" << config.probePoint.pinIndex
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
