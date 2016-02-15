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

#include <stdexcept>

namespace debug_agent
{
namespace util
{

/** Exception template to generate a unique exception per class.
 *
 * This class intends to avoid the following duplication in all class that throw:
 *     class Fuu {
 *         struct Exception : public std::runtime_error {
 *             using std::runtime_error::runtime_error;
 *         };
 *         struct SpecificException : public Exception {
 *             using Exception::Exception;
 *         };
 *     };
 *
 *  Using this template, the previous code can be rewriten as such:
 *      class Fuu {
 *          using Exception = util::Exception<Fuu>;
 *          using SpecificException = util::Exception<Fuu, Exception>;
 *      };
 *
 * @tparam Thrower The class that will throw this exception.
 * @tparam Base The standard error it should derive from.
 */
template <class Thrower, class Base = std::runtime_error>
struct Exception : public Base
{
    using Base::Base;
};

} // namespace debug_agent
} // namespace util
