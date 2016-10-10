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
template <template <class> class Traits>
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
    void pushElement(C &)
    {
        using NonConstC = typename std::remove_const<C>::type;

        Poco::AutoPtr<Poco::XML::Element> element =
            mDocument->createElement(Traits<NonConstC>::tag);

        if (mElementStack.empty()) {
            mDocument->appendChild(element);
        } else {
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

    Serializer(const Serializer &) = delete;
    Serializer &operator=(const Serializer &) = delete;

    Poco::AutoPtr<Poco::XML::Document> mDocument;
    ElementStack mElementStack;
};
}
}
}
