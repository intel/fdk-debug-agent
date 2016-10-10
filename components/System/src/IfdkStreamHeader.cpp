/*
 * Copyright (c) 2015, Intel Corporation
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

#include <System/IfdkStreamHeader.hpp>
#include <cstdint>
#include <vector>

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

std::istream &operator>>(std::istream &is, const IfdkStreamHeader &)
{
    std::vector<char> buffer(IfdkStreamHeader::formatTypeMaxLength + 1);
    is.read(buffer.data(), buffer.size());

    uint32_t nbProperties = 0;
    is.read(reinterpret_cast<char *>(&nbProperties), sizeof(nbProperties));

    auto propertyKeyMaxLength = IfdkStreamHeader::formatTypeMaxLength;
    auto propertyValueMaxLength = IfdkStreamHeader::formatTypeMaxLength;

    // toss the properties out of the stream
    for (uint32_t i = 0; i < nbProperties; ++i) {
        buffer.resize(propertyKeyMaxLength);
        is.read(buffer.data(), propertyKeyMaxLength + 1);

        buffer.resize(propertyValueMaxLength);
        is.read(buffer.data(), propertyValueMaxLength + 1);
    }

    return is;
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
