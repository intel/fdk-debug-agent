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
    std::string getInfoParameterValue(uint16_t moduleTypeId, uint16_t instanceId);
    std::string getControlParameterValue(uint16_t moduleTypeId, const std::string &moduleTypeUuid,
                                         uint16_t instanceId);
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
    std::string fdkModuleTypeToCavs(const std::string &fdkModuleType) const;

    /** Translate a FDK parameter kind into a ParameterSerializer parameter kind */
    static parameter_serializer::ParameterSerializer::ParameterKind translate(ParameterKind kind);

    /** Find module id and uuid from its fdk name */
    std::pair<uint16_t /*moduleTypeId*/, std::string /*moduleTypeUuid*/> getModuleInfo(
        const std::string &fdkModuleTypeName) const;

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
