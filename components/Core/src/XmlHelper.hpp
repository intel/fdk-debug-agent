/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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
