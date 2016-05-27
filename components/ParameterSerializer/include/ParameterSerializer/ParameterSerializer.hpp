/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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
