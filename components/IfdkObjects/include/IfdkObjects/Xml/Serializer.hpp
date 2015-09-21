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

#include <Poco/AutoPtr.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/XML/XMLWriter.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Text.h>
#include <stack>
#include <string>
#include <type_traits>
#include <sstream>

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{
/** XML Serializer base class.
 *
 * It intends to be subclassed for a given data model.
 *
 * This base serializer use type meta data (for instance xml tag name) from a trait class
 * supplied as template parameter.
 *
 * For more information about the traits format, see the Deserialize class.
 */
template<template<class> class Traits>
class Serializer
{
public:
    /* The usage of operator new cannot be avoided since needed with Poco::AutoPtr. */
    Serializer() : mDocument(new Poco::XML::Document()) {}

    /** Return the serialized xml */
    std::string getXml() const
    {
        Poco::XML::DOMWriter writer;
        writer.setNewLine("\n");
        writer.setOptions(Poco::XML::XMLWriter::PRETTY_PRINT);
        writer.setIndent("    ");

        std::stringstream output;
        writer.writeNode(output, mDocument);
        return output.str();
    }

protected:
    /** Create a DOM element and push it on the stack. This element becomes the current element.
     *
     * The xml tag name is deduced from the type using traits.
     */
    template <class C>
    void pushElement(C& e)
    {
        using NonConstC = typename std::remove_const<C>::type;

        Poco::AutoPtr<Poco::XML::Element> element =
            mDocument->createElement(Traits<NonConstC>::tag);

        if (mElementStack.empty()) {
            mDocument->appendChild(element);
        }
        else {
            topElement().appendChild(element);
        }

        mElementStack.push(element);
    }

    /** Pop a DOM element */
    void popElement()
    {
        assert(!mElementStack.empty());
        mElementStack.pop();
    }

    /** Return the current DOM element, which is at the top of the stack */
    Poco::XML::Element &topElement()
    {
        assert(!mElementStack.empty());
        return *mElementStack.top();
    }

    /* Helper class to set a xml attribute to the current node */
    void setAttribute(const std::string &name, const std::string &value)
    {
        topElement().setAttribute(name, value);
    }

    /* Helper class to set text content to the current node */
    void setText(const std::string &txt)
    {
        Poco::AutoPtr<Poco::XML::Text> textNode(mDocument->createTextNode(txt));
        topElement().appendChild(textNode);
    }

private:
    using ElementStack = std::stack<Poco::XML::Element *>;

    Serializer(const Serializer&) = delete;
    Serializer& operator = (const Serializer&) = delete;

    Poco::AutoPtr<Poco::XML::Document> mDocument;
    ElementStack mElementStack;
};

}
}
}


