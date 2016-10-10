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
 * @code
 * class A;
 * class B: public A;
 * class C: public A;
 * @endcode
 *
 * Matching traits are:
 *
 * @code
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
 * @endcode
 *
 * Then this code :
 *
 * @code
 * A *instance = DynamicFactory<Traits>::createInstanceFromTag<A, B, C>("B");
 * @endcode
 *
 * the "instance" local variable will contain a B class instance.
 *
 * @tparam Traits the traits class use to retrieve type tags.
 */
template <template <class> class Traits>
class DynamicFactory final
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
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
