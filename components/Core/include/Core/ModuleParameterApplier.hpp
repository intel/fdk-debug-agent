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

#include "Core/ParameterApplier.hpp"
#include "cAVS/System.hpp"
#include "Util/Locker.hpp"
#include "ParameterSerializer/ParameterSerializer.hpp"
#include <map>

namespace debug_agent
{
namespace core
{

/** Applies FDK parameters to cAVS modules */
class ModuleParameterApplier : public ParameterApplier
{
public:
    ModuleParameterApplier(
        cavs::System &system,
        util::Locker<parameter_serializer::ParameterSerializer> &parameterSerializer);

    std::set<std::string> getSupportedTypes() const override;

    std::string getParameterStructure(const std::string &type, ParameterKind kind) override;

    std::string getParameterValue(const std::string &type, ParameterKind kind,
                                  const std::string &instanceId) override;

    void setParameterValue(const std::string &type, ParameterKind kind,
                           const std::string &instanceId,
                           const std::string &parameterValue) override;

private:
    /** Returns the xml tag that matches a parameter kind (info, control) */
    static std::string getParameterKindTag(ParameterKind parameterKind);

    /** Returns an XPath that points to structure root node of the supplied parameter name */
    static std::string getParameterXPath(ParameterKind parameterKind, const std::string &paramName);

    /** Parse a module instance id from string
     * @throws ParameterApplier::Exception if the parsing fails
     */
    static uint16_t parseModuleInstanceId(const std::string &instanceIdStr);

    /** Converts a fdk module type name to a cAVS module type name
     * - fdk module type name example: "cavs.module-aec"
     * - cavs module type name example: "aec"
     */
    std::string fdkModuleTypeToCavs(const std::string &fdkModuleType);

    /** Translate a FDK parameter kind into a ParameterSerializer parameter kind */
    static parameter_serializer::ParameterSerializer::ParameterKind translate(ParameterKind kind);

    /** Find module type id from its name */
    uint16_t getModuleTypeId(const std::string &moduleTypeName);

    /** Find a parameter id from its name */
    cavs::dsp_fw::ParameterId getParamIdFromName(const std::string moduleTypeName,
                                                 ParameterKind parameterKind,
                                                 const std::string parameterName);

    /** @return tjhe list of parameter names of a given module type */
    std::vector<std::string> getModuleParameterNames(const std::string &moduleTypeName,
                                                     ParameterKind parameterKind) const;

    cavs::System &mSystem;
    util::Locker<parameter_serializer::ParameterSerializer> &mParameterSerializer;
    std::map<std::string, std::string> mFdkToCavsModuleNames;
};
}
}
