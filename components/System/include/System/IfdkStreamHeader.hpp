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
    class Exception final : public std::logic_error
    {
    public:
        explicit Exception(const std::string &what) : std::logic_error(what) {}
    };

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