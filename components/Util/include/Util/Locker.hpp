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

#include <mutex>
#include <utility>

namespace debug_agent
{
namespace util
{

/** Wraps an object to make each method call atomic
 *
 * Methods of the original object can be accessed through the -> operator and
 * each method call will lock the whole object.
 *
 * Usage example:
 *
 *     int f(Locker &critical)
 *     {
 *         int a = critical->method1(); // atomic call w.r.t. the "critical" object
 *         a += critical->method2(); // atomic call w.r.t. the "critical" object
 *
 *         return a;
 *     }
 *
 * @tparam T The class to be wrapped
 */
template<class T>
class Locker final
{
public:
    /*
     * This class has to have its own template in order to selectively behave
     * as const or non-const, according to the constness of the Locker object.
     *
     * Having the Locker class return a const Guard would not work because that
     * object may need to be either moved or copied by the caller but:
     *  - unique_lock can't be copied.
     *  - the semantics of moving from a const is unclear.
     */
    template<class U>
    class Guard final
    {
    public:
        /** Access the wrapped object
         */
        U *operator->() const { return &mWrapped; }

    private:
        friend Locker<T>;
        Guard(U &wrapped, std::mutex &mutex) : mWrapped(wrapped), mLock(mutex) {}

        U &mWrapped;
        std::unique_lock<std::mutex> mLock;
    };

    template <class... Args>
    Locker(Args &&... args) : mWrapped(std::forward<Args>(args)...) {}

    /** Call a method on the wrapped object, atomically
     *
     * @{
     */
    /*
     * Takes advantage on the automatic -> recursive call provided by C++: if
     * an operator-> doesn't return a pointer, operator-> is called on the
     * returned object, and so on. In our case, Guard::operator->() returns a
     * Guard object and Guard::operator->() will be automatically called on
     * it and return a T*.
     *
     * See
     * http://en.cppreference.com/w/cpp/language/operator_member_access#Built-in_member_access_operators
     * (§3 at the time of writing).
     */
    Guard<T> operator->() {
        return { mWrapped, mMutex };
    }

    Guard<const T> operator->() const {
        return { mWrapped, mMutex };
    }
    /** @} */

private:
    mutable std::mutex mMutex;
    T mWrapped;
};

} /* namespace util */
} /* namespace debug_agent */
