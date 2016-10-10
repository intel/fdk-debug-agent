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
 * Usage example for atomic method call:
 *
 *     int f(Locker &critical)
 *     {
 *         int a = critical->method1(); // atomic call w.r.t. the "critical" object
 *         a += critical->method2(); // atomic call w.r.t. the "critical" object
 *
 *         return a;
 *     }
 *
 * Usage example for atomic sessions:
 *
 *
 *     void f(Locker &critical)
 *     {
 *         int a;
 *
 *         { // critical section
 *             auto locked = critical.lock();
 *
 *             a = locked->method1();
 *             a += locked->method2();
 *         }
 *
 *         a += 1;
 *         return a;
 *     }
 *
 * @tparam T The class to be wrapped
 */
template <class T>
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
    template <class U>
    class Guard final
    {
    public:
        /** Access the wrapped object
         *
         * @{
         */
        U *operator->() const { return &mWrapped; }

        U *get() const { return &mWrapped; }
        /** @} */

    private:
        friend Locker<T>;
        Guard(U &wrapped, std::mutex &mutex) : mWrapped(wrapped), mLock(mutex) {}

        U &mWrapped;
        std::unique_lock<std::mutex> mLock;
    };

    template <class... Args>
    Locker(Args &&... args) : mWrapped(std::forward<Args>(args)...)
    {
    }

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
    Guard<T> operator->() { return {mWrapped, mMutex}; }

    Guard<const T> operator->() const { return {mWrapped, mMutex}; }

    /** @} */

    /** Lock and return a guard wrapper around the underlying object
     *
     * The underlying object is unlocked when the guard wrapper is destroyed.
     * This allows for scoped locking, using the same idiom as std::lock_guard.
     *
     * @{
     */
    Guard<T> lock() { return {mWrapped, mMutex}; }

    Guard<const T> lock() const { return {mWrapped, mMutex}; }
    /** @} */

private:
    mutable std::mutex mMutex;
    T mWrapped;
};

} /* namespace util */
} /* namespace debug_agent */
