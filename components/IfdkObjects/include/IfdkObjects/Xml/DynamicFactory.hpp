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

#include <Poco/DOM/Node.h>
#include <Poco/DOM/Element.h>
#include <stdexcept>
#include <memory>

namespace debug_agent
{
namespace ifdk_objects
{
namespace xml
{

/** This factory instantiates a class using a tag name. The instanciated class must
 * derived from a base class supplied as template parameter.
 *
 * The tag name is associated to class using traits.
 *
 * For instance Consider this class hierarchy:
 * class A;
 * class B: public A;
 * class C: public A;
 *
 * Matching traits are:
 *
 * template <class T>
 * struct Traits
 * {
 * }
 *
 * template<>
 * struct Traits<B>
 * {
 *    std::string tag = "B";
 * }
 *
 * template<>
 * struct Traits<C>
 * {
 *    std::string tag = "C";
 * }
 *
 * Then this code :
 *
 * A *instance = DynamicFactory<Traits>::createInstanceFromTag<A, B, C>("B");
 *
 * the "instance" local variable will contain a B class instance.
 *
 * @tparam Traits the traits class use to retrieve type tags.
 */
template <template <class> class Traits>
class DynamicFactory final
{
public:
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    /** Instantiates a class using a tag name
     *
     * @tparam Base the base class of all instantiable types
     * @tparam OtherDerived all instantiable types
     */
    template <class Base, class Derived, class... OtherDerived>
    static std::shared_ptr<Base> createInstanceFromTag(const std::string &name)
    {
        /* If the current type matches the tag, create an instance of that type */
        if (Traits<Derived>::tag == name) {
            return std::make_shared<Derived>();
        }

        /* Otherwise recurse on the next type */
        return createInstanceFromTag<Base, OtherDerived...>(name);
    }

private:
    DynamicFactory() = delete;

    /** Called if all types have been iterated, i.e. no class matches the supplied tag */
    template <class Base>
    static std::shared_ptr<Base> createInstanceFromTag(const std::string &name)
    {
        throw Exception("No class matches the '" + name + "' tag.");
    }
};
}
}
}
