/*
 * Copyright (c) 2015-2016, Intel Corporation
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

#include "IfdkObjects/VisitablePtrVector.hpp"
#include "IfdkObjects/Xml/DynamicFactory.hpp"
#include <Poco/AutoPtr.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Text.h>
#include <Poco/DOM/DOMParser.h>
#include <stack>
#include <string>
#include <stdexcept>

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

/** XML Deserializer base class.
 *
 * It intends to be subclassed for a given data model.
 *
 * This base deserializer uses type meta data (for instance xml tag name) from a trait class
 * supplied as template parameter.
 *
 * For instance consider this model:
 * @code
 * class A
 * {
 *    attributeA
 * }
 *
 * class B
 * {
 *    attributeB
 * }
 * @endcode
 *
 * Here an example of matching traits:
 *
 * @code
 * template <class T>
 * struct Traits
 * {
 * }
 *
 * template<>
 * struct Traits<A>
 * {
 *    std::string tag = "A";
 * }
 *
 * template<>
 * struct Traits<B>
 * {
 *    std::string tag = "B";
 * }
 * @endcode
 */
template <template <class> class Traits>
class Deserializer
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** Create a deserializer from a supplied xml string */
    Deserializer(const std::string &xml)
    {
        Poco::XML::DOMParser parser;
        try {
            mDocument = parser.parseString(xml);
        } catch (Poco::Exception &e) {
            throw Exception("Unable to parse xml string: " + std::string(e.what()));
        }
    }

protected:
    /** Find a child of the current DOM element that matches the supplied type, and push it on
     * the stack. This element becomes the current element.
     *
     * The xml tag name is deduced from the type using traits.
     */
    template <class T>
    void pushElement(T &)
    {
        /* Getting type tag name using traits */
        std::string name = Traits<T>::tag;

        Poco::XML::Element *element;
        if (mElementStack.empty()) {
            /* If the stack is empty, take the first DOM node of the document.*/
            Poco::XML::Node *documentChild = mDocument->firstChild();

            /* Check that the node tag match the serialized type*/
            if (documentChild->nodeType() != Poco::XML::Node::ELEMENT_NODE ||
                documentChild->nodeName() != name) {
                throw Exception("Wrong root element name: '" + documentChild->nodeName() +
                                "' instead of '" + name + "'");
            }

            element = static_cast<Poco::XML::Element *>(documentChild);
        } else {
            /* If the stack is not empty, find the first child with matching tag. */
            element = topElement().getChildElement(name);
            if (element == nullptr) {
                throw Exception("Element '" + name + "' not found in parent '" +
                                topElement().nodeName() + "'");
            }
        }

        mElementStack.push(element);
    }

    /** Pop a DOM element from stack
     *
     * The DOM elemement is alse remove from the document, in this way it will not be visited
     * twice.
     */
    void popElement()
    {
        assert(!mElementStack.empty());

        /* Removing the element from the DOM document */
        Poco::XML::Element *element = &topElement();
        Poco::XML::Node *parentNode = element->parentNode();
        parentNode->removeChild(element);

        /* Removing the element from the stack */
        mElementStack.pop();
    }

    /** Helper method to get an attribute of the current DOM element */
    std::string getStringAttribute(const std::string &attributeName)
    {
        const Poco::XML::Element &element = topElement();
        if (!element.hasAttribute(attributeName)) {
            throw Exception("The required attribute '" + attributeName +
                            "' has not been found in element '" + element.nodeName() + "'");
        }
        return element.getAttribute(attributeName);
    }

    /** Helper method to get text content of a DOM element */
    std::string getText()
    {
        Poco::XML::Text *text = getFirstChild<Poco::XML::Text>(Poco::XML::Node::TEXT_NODE);
        if (text == nullptr) {
            /* No text node found: text is empty */
            return "";
        }
        return text->getNodeValue();
    }

    /** Return the current DOM element, which is at the top of the stack */
    Poco::XML::Element &topElement()
    {
        assert(!mElementStack.empty());
        return *mElementStack.top();
    }

    /** Return the child element count of the top DOM element.
     */
    std::size_t getChildElementCount()
    {
        std::size_t count = 0;
        for (Poco::XML::Node *node = topElement().firstChild(); node != nullptr;
             node = node->nextSibling()) {
            if (node->nodeType() == Poco::XML::Node::ELEMENT_NODE) {
                count++;
            }
        }
        return count;
    }

    /** Fill a polymporphic list from DOM element children.
     *
     * For instance consider this xml, that uses a polymorphic list:
     *
     * @code
     * <parameters>
     *    <int_parameter name="p1">1</int_parameter>
     *    <float_parameter name="p1">1.2</float_parameter>
     * </parameters>
     * @endcode
     *
     * The matching data model is:
     *
     * @code
     * class Parameter
     * {
     *     std::string name;
     * }
     *
     * class IntParameter : public Parameter
     * {
     *     int intValue;
     * }
     *
     * class FloatParameter : public Parameter
     * {
     *     float floatValue;
     * }
     * @endcode
     *
     * using Parameters = std::vector<std::shared_ptr<Parameter>>;
     *
     * Now if you invoke this code with the previous XML content:
     *
     * @code
     *   Parameters parameters;
     *   fillPolymorphicVector<Parameter, IntParameter, FloatParameter>(parameters);
     * @endcode
     *
     *
     * Then the parameter list will be filled with an IntParameter instance and with a
     * FloatParameter instance.
     *
     * @tparam Base the base class of all intantiables types.
     * @tparam Derived type list of all intantiables types.
     */
    template <class Base, class... Derived>
    void fillPolymorphicVector(std::vector<std::shared_ptr<Base>> &vector)
    {
        /* Iterating on element children*/
        for (Poco::XML::Node *node = topElement().firstChild(); node != nullptr;
             node = node->nextSibling()) {
            if (node->nodeType() == Poco::XML::Node::ELEMENT_NODE) {

                try {
                    /* Creating the instance that matches the tag */
                    std::shared_ptr<Base> instance =
                        DynamicFactory<Traits>::template createInstanceFromTag<Base, Derived...>(
                            node->nodeName());
                    if (instance == nullptr) {
                        throw Exception("Invalid type ref name: " + node->nodeName());
                    }

                    vector.push_back(instance);
                } catch (typename DynamicFactory<Traits>::Exception &e) {
                    throw Exception("Unable to instanciate class from tag '" + node->nodeName() +
                                    "' : " + std::string(e.what()));
                }
            }
        }
    }

private:
    using ElementStack = std::stack<Poco::XML::Element *>;

    Deserializer(const Deserializer &) = delete;
    Deserializer &operator=(const Deserializer &) = delete;

    /** @return the first node child of a given type */
    template <class T>
    T *getFirstChild(int nodeType)
    {
        for (Poco::XML::Node *node = topElement().firstChild(); node != nullptr;
             node = node->nextSibling()) {
            if (node->nodeType() == nodeType) {
                return static_cast<T *>(node);
            }
        }
        return nullptr;
    }

    Poco::AutoPtr<Poco::XML::Document> mDocument;
    ElementStack mElementStack;
};
}
}
}
