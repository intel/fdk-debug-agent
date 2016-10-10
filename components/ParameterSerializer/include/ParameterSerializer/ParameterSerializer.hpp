/*
 * Copyright (c) 2015-2016, Intel Corporation
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
#pragma once

#include "Util/EnumHelper.hpp"
#include "Util/Buffer.hpp"
#include "Util/Exception.hpp"
#include <string>
#include <stdexcept>
#include <vector>
#include <map>
#include <memory>

/* Forward declaration of internal types */
class CParameterMgrPlatformConnector;
class ElementHandle;

namespace debug_agent
{
namespace parameter_serializer
{
/**
 * Parameter Serializer
 */
class ParameterSerializer
{
public:
    using Exception = util::Exception<ParameterSerializer, std::logic_error>;
    using ElementNotFound = util::Exception<ParameterSerializer>;

    /**
     * Constructor
     * @param[in] configurationFilePath path to the XML configuration file of the pfw instance
     * @param[in] validationRequested flag used to enable the validation of the XML configuration
     *            files
     */
    ParameterSerializer(const std::string configurationFilePath, bool validationRequested = false);

    /**
     * Destructor implementation shall not be inlined because of the use of unique_ptr on
     * parameter-framework plateform conector.
     */
    virtual ~ParameterSerializer();

    /**
     * Parameter kind enumerate
     */
    enum class ParameterKind
    {
        Info,
        Control
    };

    /** enum helper (string conversion, type validation) for ParameterKind enum  */
    static const util::EnumHelper<ParameterKind> &parameterKindHelper()
    {
        static util::EnumHelper<ParameterKind> helper({
            {ParameterKind::Info, "info"}, {ParameterKind::Control, "control"},
        });
        return helper;
    }

    /**
     * This method returns the children of the info/control subnode an element.
     * @param[in] subsystemName is the name of the subsystem (eg. cavs)
     * @param[in] elementName is the name of the element
     * @param[in] parameterKind is the kind of subnode (eg. control or info)
     * @return a map containing the childId and its corresponding name of children of the element
     * @throw ParameterSerializer::Exception or ParameterSerializer::ElementNotFound
     */
    std::map<uint32_t, std::string> getChildren(const std::string &subsystemName,
                                                const std::string &elementName,
                                                ParameterKind parameterKind) const;

    /**
     * This method returns the the mapping of a parameter.
     * @param[in] subsystemName is the name of the subsystem (eg. cavs)
     * @param[in] elementName is the name of the element
     * @param[in] parameterKind is the kind of subnode (eg. control or info)
     * @param[in] parameterName is the name of the parameter whoze mapping will be returned
     * @param[in] key is the key to retrieve corresponding mapping
     * @return a string containing the mapping of a parameter.
     * @throw ParameterSerializer::Exception
     */
    std::string getMapping(const std::string &subsystemName, const std::string &elementName,
                           ParameterKind parameterKind, const std::string &parameterName,
                           const std::string &key) const;

    /**
     * This method serializes an XML string containing the settings of one parameter (child)
     * of an element in a binary format.
     * @param[in] subsystemName is the name of the subsystem (eg. cavs)
     * @param[in] elementName is the name of the element
     * @param[in] parameterKind is the kind of subnode (eg. control or info)
     * @param[in] parameterName is the name of the parameter to serialize
     * @param[in] parameterAsXml is a string contaning the parameter settings in XML format
     * @return a binary payload containing the parameter settings
     * @throw ParameterSerializer::Exception
     */
    util::Buffer xmlToBinary(const std::string &subsystemName, const std::string &elementName,
                             ParameterKind parameterKind, const std::string &parameterName,
                             const std::string parameterAsXml) const;

    /**
     * This method serializes binary payload containing the settings of one parameter (child)
     * of an element in an XML string.
     * @param[in] subsystemName is the name of the subsystem (eg. cavs)
     * @param[in] elementName is the name of the element
     * @param[in] parameterKind is the kind of subnode (eg. control or info)
     * @param[in] parameterName is the name of the parameter to serialize
     * @param[in] parameterPayload a binary payload containing the parameter settings
     * @return a string contaning the parameter settings in XML format
     * @throw ParameterSerializer::Exception
     */
    std::string binaryToXml(const std::string &subsystemName, const std::string &elementName,
                            ParameterKind parameterKind, const std::string &parameterName,
                            const util::Buffer &parameterPayload) const;

    /**
     * This method returns the structure of a parameter
     * @param[in] subsystemName is the name of the subsystem (eg. cavs)
     * @param[in] elementName is the name of the element
     * @param[in] parameterKind is the kind of subnode (eg. control or info)
     * @param[in] parameterName is the name of the parameter whose structure will be returned
     * @return a string contaning the parameter structure in XML format
     * @throw ParameterSerializer::Exception
     */
    std::string getStructureXml(const std::string &subsystemName, const std::string &elementName,
                                ParameterKind parameterKind,
                                const std::string &parameterName) const;

private:
    std::unique_ptr<ElementHandle> getElement(const std::string &subsystemName,
                                              const std::string &elementName,
                                              ParameterKind parameterKind) const;

    std::unique_ptr<ElementHandle> getChildElementHandle(const std::string &subsystemName,
                                                         const std::string &elementName,
                                                         ParameterKind parameterKind,
                                                         const std::string &parameterName) const;

    /**
     * Raise an exception if the Parameter Platform Connector is not correctly instantiated nor
     * started.
     * @throw ParameterSerializer::Exception
     */
    void checkParameterMgrPlatformConnector() const;

    static void stripFirstLine(std::string &document);

    std::unique_ptr<CParameterMgrPlatformConnector> mParameterMgrPlatformConnector;
};
}
}
