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
