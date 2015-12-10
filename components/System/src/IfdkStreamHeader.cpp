/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
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

#include <System/IfdkStreamHeader.hpp>
#include <cstdint>

namespace debug_agent
{
namespace system
{

const std::string IfdkStreamHeader::ifdk("IFDK");
const size_t IfdkStreamHeader::propertyKeyMaxLength = 31;
const size_t IfdkStreamHeader::propertyValueMaxLength = 31;
const size_t IfdkStreamHeader::formatTypeMaxLength = 31;

IfdkStreamHeader::IfdkStreamHeader(const std::string &systemType, const std::string &fileType,
                                   unsigned int majorVersion, unsigned int minorVersion)
    : mFormatType()
{
    if (systemType.length() == 0) {

        throw Exception("Empty System Type string");
    }
    if (fileType.length() == 0) {

        throw Exception("Empty File Type string");
    }

    /* Format is "IFDK:<system type>:<file type>[major version.minor version]" */
    mFormatType += ifdk + ':' + systemType + ':' + fileType;
    mFormatType += '[' + std::to_string(majorVersion) + '.' + std::to_string(minorVersion) + ']';

    if (mFormatType.length() > formatTypeMaxLength) {

        throw Exception("Format type length exceeds " +
                        std::to_string(IfdkStreamHeader::formatTypeMaxLength) +
                        " characters for '" + mFormatType + "'");
    }
}

void IfdkStreamHeader::addProperty(const std::string &key, const std::string &value)
{
    const std::string equal("=");

    if (key.length() > propertyKeyMaxLength || value.length() > propertyValueMaxLength) {

        throw Exception("Property length exceeds for " + key + equal + value);
    } else if (key.length() == 0 || value.length() == 0) {

        throw Exception("Empty property key or value for " + key + equal + value);
    }

    std::pair<IfdkStreamHeader::Properties::iterator, bool> ret;
    ret = mProperties.insert(Property(key, value));

    if (ret.second == false) {

        throw Exception("Property already exists for key '" + key + "'");
    }
}

std::ostream &operator<<(std::ostream &os, const IfdkStreamHeader &header)
{
    /* Streams out format padding with '\0' up to formatTypeMaxLength+1 bytes */
    std::string copyFormatType(header.mFormatType);
    copyFormatType.resize(header.formatTypeMaxLength + 1, '\0');
    os.write(copyFormatType.c_str(), copyFormatType.size());

    /**
     * Streams out Number of properties on 32bits little endian
     * @warning this code is designed for little endian machine like x86 and will not produce
     * correct format on big endian machine. */
    uint32_t nbProperties = static_cast<uint32_t>(header.mProperties.size());
    os.write(reinterpret_cast<char *>(&nbProperties), sizeof(nbProperties));

    /* Streams out each property */
    for (auto property : header.mProperties) {

        /* padding key and value with '\0' up to <key|value>TypeMaxLength+1 bytes */
        std::string copyPropertyKey(property.first);
        copyPropertyKey.resize(header.propertyKeyMaxLength + 1, '\0');
        std::string copyPropertyValue(property.second);
        copyPropertyValue.resize(header.propertyValueMaxLength + 1, '\0');

        os.write(copyPropertyKey.c_str(), copyPropertyKey.size());
        os.write(copyPropertyValue.c_str(), copyPropertyValue.size());
    }

    return os;
}
}
}