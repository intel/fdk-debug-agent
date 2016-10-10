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

#pragma once

#include "Util/AssertAlways.hpp"
#include "Util/EnumHelper.hpp"
#include <Poco/AutoPtr.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Node.h>
#include <Poco/DOM/Document.h>
#include <Poco/XML/XML.h>
#include <Poco/XML/XMLString.h>
#include <stdexcept>
#include <string>
#include <sstream>
#include <typeinfo>

namespace debug_agent
{
namespace core
{

/** Class that helps to manipulate xml files.
 *
 * Note: Normally XML files are parsed using a serialization framework that mirrors xml content
 * to a specified data model.
 *
 * But currently the serialization of parameter data model does not exists yet, it will be provided
 * by the libstructure component.
 *
 * So this class is used to query xml values until the libstructure is available.
 *
 * XML parsing is done using Poco::Xml;
 *
 * @todo: remove this class when libstructure is available
 */
class XmlHelper
{
public:
    class Exception : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    /** Construct an xml helper from a xml string */
    XmlHelper(const std::string &xmlStr)
    {
        try {
            Poco::XML::DOMParser parser;
            mDocument = parser.parseString(xmlStr);
            ASSERT_ALWAYS(!mDocument.isNull());
        } catch (Poco::Exception &e) {
            throw Exception("Can not parse xml: " + std::string(e.what()) + "\nString source:\n" +
                            xmlStr);
        }
    }

    /** Fetch a value as string using a XPath */
    std::string getValueFromXPathAsString(const std::string &path)
    {
        try {
            // ownership is not transferred
            Poco::XML::Node *node = mDocument->getNodeByPath(path);
            if (node != nullptr) {
                return Poco::XML::fromXMLString(node->innerText());
            } else {
                throw Exception("Node not found with xpath " + path);
            }
        } catch (Poco::Exception &e) {
            throw Exception("Cannot get xpath value: " + std::string(e.what()) + "\nXPath: " +
                            path);
        }
    }

    /** Fetch a value as enum using a XPath, using an util::EnumHelper instance. */
    template <typename EnumT>
    EnumT getValueFromXPathAsEnum(const std::string &path,
                                  const util::EnumHelper<EnumT> &enumHelper)
    {
        std::string valueStr = getValueFromXPathAsString(path);
        EnumT value;
        if (!enumHelper.fromString(valueStr, value)) {
            throw Exception("Enum '" + std::string(typeid(value).name()) + "' : invalid value '" +
                            valueStr + "'");
        }
        return value;
    }

    /** Fetch a value as a template type using a XPath */
    template <typename T>
    T getValueFromXPath(const std::string &path)
    {
        std::string valueStr = getValueFromXPathAsString(path);
        std::stringstream stream(valueStr);
        T value;
        stream >> value;
        if (stream.fail()) {
            throw Exception("Can not convert string '" + valueStr + "' into " +
                            typeid(value).name());
        }
        return value;
    }

    /** Return a subtree of the whole xml tree. Subtree root is provided through a XPath. */
    std::string getSubTree(const std::string &path)
    {
        Poco::AutoPtr<Poco::XML::Document> childDocument = new Poco::XML::Document;
        childDocument->appendChild(childDocument->importNode(mDocument->getNodeByPath(path), true));

        Poco::XML::DOMWriter writer;
        std::stringstream out;
        writer.writeNode(out, childDocument);
        return out.str();
    }

private:
    XmlHelper(const XmlHelper &) = delete;
    XmlHelper &operator=(const XmlHelper &) = delete;

    Poco::AutoPtr<Poco::XML::Document> mDocument;
};
}
}
