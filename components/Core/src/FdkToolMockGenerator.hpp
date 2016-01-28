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

#include "Core/TypeModel.hpp"
#include "Core/InstanceModel.hpp"
#include "IfdkObjects/Xml/TypeSerializer.hpp"
#include "IfdkObjects/Xml/InstanceSerializer.hpp"
#include <Poco/Zip/Compress.h>
#include <Poco/Zip/ZipException.h>
#include <Poco/MemoryStream.h>
#include <sstream>
#include <stdexcept>

namespace debug_agent
{
namespace core
{

/** This class creates fdk tool mock files from the model cache
 *
 * Mock files are delivered into a zip archive.
 */
class FdkToolMockGenerator
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    FdkToolMockGenerator(std::ostream &outputStream) : mCompress(outputStream, true) {}

    ~FdkToolMockGenerator() { close(); }

    /** Write type model in archive */
    void setTypeModel(const TypeModel &typeModel)
    {
        try {
            writeValue<ifdk_objects::xml::TypeSerializer>(*typeModel.getSystem(), "/type");

            for (auto it : typeModel.getTypeMap()) {
                writeValue<ifdk_objects::xml::TypeSerializer>(*(it.second), "/type/" + it.first);
            }
        } catch (Poco::Zip::ZipException &e) {
            throw Exception("Zip error: " + std::string(e.what()));
        }
    }

    /** Write system instance in archive */
    void setSystemInstance(const ifdk_objects::instance::System &system)
    {
        try {
            writeValue<ifdk_objects::xml::InstanceSerializer>(system, "/instance");
        } catch (Poco::Zip::ZipException &e) {
            throw Exception("Zip error: " + std::string(e.what()));
        }
    }

    /* Write instance model in archive */
    void setInstanceModel(const InstanceModel &instanceModel)
    {
        try {
            for (auto &entry : instanceModel.getCollectionMap()) {
                const ifdk_objects::instance::BaseCollection &collection = *(entry.second);

                writeValue<ifdk_objects::xml::InstanceSerializer>(collection,
                                                                  "/instance/" + entry.first);

                std::vector<std::shared_ptr<const ifdk_objects::instance::Instance>> instances;
                collection.getInstances(instances);

                for (auto &entry2 : instances) {
                    writeValue<ifdk_objects::xml::InstanceSerializer>(
                        *entry2, "/instance/" + entry.first + "/" + entry2->getInstanceId());
                }
            }
        } catch (Poco::Zip::ZipException &e) {
            throw Exception("Zip error: " + std::string(e.what()));
        }
    }

    void close() { mCompress.close(); }

private:
    /** Write a file in the zip archive using the url as path */
    void writeUrlContent(const std::string &url, const std::string &content)
    {
        std::string filePath("model" + url + ".xml");

        std::istringstream stream(content);
        mCompress.addFile(stream, Poco::DateTime(), filePath);
    }

    template <class Serializer, class Value>
    void writeValue(Value &value, const std::string &uri)
    {
        Serializer serializer;
        value.accept(serializer);
        writeUrlContent(uri, serializer.getXml());
    }

    Poco::Zip::Compress mCompress;
};
}
}
