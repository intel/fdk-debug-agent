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
#pragma once

#include <string>
#include <map>
#include <stdexcept>
#include <iostream>

namespace debug_agent
{
namespace system
{

/**
 * The IfdkStreamHeader class describes the header of a IFDK stream provided by a FDK service.
 */
class IfdkStreamHeader final
{
public:
    struct Exception final : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** When deserializing, IfdkStreamHeader is two-step initialized.
     */
    IfdkStreamHeader() = default;

    /**
     * @param[in] systemType the name of the system type that provides this stream
     * @param[in] fileType the name of the file type of this stream
     * @param[in] majorVersion the major version of the file type stream format
     * @param[in] minorVersion the minor version of the file type stream format
     * @throw IfdkStreamHeader::Exception
     */
    explicit IfdkStreamHeader(const std::string &systemType, const std::string &fileType,
                              unsigned int majorVersion, unsigned int minorVersion);

    /**
     * Add a property to the stream.
     * A key is composed of a key and a value, both being strings.
     * Properties are optional and may be added in a dedicated section of the stream.
     * The method raises an exception if the added property key is already present.
     * @param[in] key The property key to be added
     * @param[in] value The property value to be added for the given key
     * @throw IfdkStreamHeader::Exception
     */
    void addProperty(const std::string &key, const std::string &value);

    /**
     * Serialize the header to a ostream
     * @param[in] os the ostream
     * @param[in] header the IfdkStreamHeader to be serialized
     * @return ostream containing original ostream plus serialized IfdkStreamHeader
     */
    friend std::ostream &operator<<(std::ostream &os, const IfdkStreamHeader &header);
    /**
     * Deserialize an istream into an ifdk header
     * @param[in] is the istream
     * @param[in] header the IfdkStreamHeader to be deserialized
     * @return istream containing original ostream minus deserialized IfdkStreamHeader
     */
    friend std::istream &operator>>(std::istream &is, const IfdkStreamHeader &header);

private:
    std::string mFormatType;

    using Properties = std::map<std::string, std::string>;
    using Property = std::pair<std::string, std::string>;
    Properties mProperties;

    static const std::string ifdk;
    static const size_t propertyKeyMaxLength;
    static const size_t propertyValueMaxLength;
    static const size_t formatTypeMaxLength;
};
}
}
