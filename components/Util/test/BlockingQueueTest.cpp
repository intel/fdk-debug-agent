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
#include "Util/BlockingQueue.hpp"
#include <catch.hpp>
#include <future>

using namespace debug_agent::util;

/* A dummy structure */
struct Test
{
    Test(std::size_t size) : mSize(size) {}

    std::size_t mSize;
};

/* Returns the virtual size of the dummy structure */
std::size_t sizeTest(const Test &test)
{
    return test.mSize;
}

using TestPtr = std::unique_ptr<Test>;
using TestQueue = BlockingQueue<Test>;

std::unique_ptr<Test> makeTest(std::size_t size)
{
    return std::make_unique<Test>(size);
}

TEST_CASE("blocking queue: simple monothread usage")
{
    TestQueue queue(5, &sizeTest);

    queue.open();

    // enqueue element
    CHECK(queue.add(makeTest(5)));
    queue.close();
    CHECK_FALSE(queue.add(makeTest(2)));

    // removing element
    TestPtr element = queue.remove();
    REQUIRE(element != nullptr);
    CHECK(element->mSize == 5);

    // removing again: queue is closed (returns nullptr)
    CHECK(queue.remove() == nullptr);
}

TEST_CASE("blocking queue: dropping")
{
    TestQueue queue(10, &sizeTest); /* Maximum memory size: 10 bytes */

    queue.open();

    /* adding 2 bytes, -> current size: 2*/
    CHECK(queue.add(makeTest(2)));

    /* adding 5 bytes, -> current size: 7*/
    CHECK(queue.add(makeTest(5)));

    /* adding 3 bytes, -> current size: 10*/
    CHECK(queue.add(makeTest(3)));

    /* Here maximum size is reached, the next add() should fail */

    /* Checking queue attributes */
    CHECK(queue.getElementCount() == 3);
    CHECK(queue.getMemorySize() == 10);

    /* Will be dropped ...*/
    CHECK_FALSE(queue.add(makeTest(1)));

    /* Checking that queue attributes have not changed */
    CHECK(queue.getElementCount() == 3);
    CHECK(queue.getMemorySize() == 10);
}

TEST_CASE("blocking queue: clearing")
{
    TestQueue queue(10, &sizeTest);

    queue.open();

    CHECK(queue.add(makeTest(2)));
    CHECK(queue.add(makeTest(5)));

    queue.clear();
    CHECK(queue.getElementCount() == 0);
}

TEST_CASE("blocking queue: multithread closing")
{
    TestQueue queue(10, &sizeTest);

    queue.open();

    /* Closing in another thread */
    std::future<void> result(std::async(std::launch::async, [&]() { queue.close(); }));

    TestPtr element(queue.remove());
    CHECK(element == nullptr);

    result.get();
}

TEST_CASE("blocking queue: multi theading usage")
{
    TestQueue queue(10, &sizeTest);

    queue.open();

    /* Performing add in another thread */
    std::future<bool> futureResult(std::async(std::launch::async, [&]() {
        if (!queue.add(makeTest(2))) {
            return false;
        }
        if (!queue.add(makeTest(5))) {
            return false;
        }
        if (!queue.add(makeTest(3))) {
            return false;
        }
        queue.close();
        return true;
    }));

    /* Consuming elements in the current thread */
    TestPtr element;
    element = queue.remove();
    REQUIRE(element != nullptr);
    CHECK(element->mSize == 2);

    element = queue.remove();
    REQUIRE(element != nullptr);
    CHECK(element->mSize == 5);

    element = queue.remove();
    REQUIRE(element != nullptr);
    CHECK(element->mSize == 3);

    /* Closed: last element is null */
    element = queue.remove();
    CHECK(element == nullptr);

    /* Checking that adding element is successful */
    CHECK(futureResult.get());
}

TEST_CASE("blocking queue: opening/closing two times")
{
    TestQueue queue(5, &sizeTest);

    // checking that queue is closed
    CHECK_FALSE(queue.isOpen());
    CHECK(queue.getElementCount() == 0);
    CHECK(queue.remove() == nullptr);
    CHECK_FALSE(queue.add(makeTest(1)));

    // opening
    queue.open();

    // enqueue 2 elements
    CHECK(queue.add(makeTest(1)));
    CHECK(queue.add(makeTest(2)));

    // closing
    queue.close();

    // removing one element, one remains in the queue
    auto element = queue.remove();
    REQUIRE(element != nullptr);
    CHECK(element->mSize == 1);
    CHECK(queue.getElementCount() == 1);

    // opening again
    queue.open();

    // enqueue 1 element
    CHECK(queue.add(makeTest(3)));

    // closing
    queue.close();

    // removing 2nd element
    element = queue.remove();
    REQUIRE(element != nullptr);
    CHECK(element->mSize == 2);

    // removing 3rd element
    element = queue.remove();
    REQUIRE(element != nullptr);
    CHECK(element->mSize == 3);

    // removing again: queue is closed (returns nullptr)
    CHECK(queue.remove() == nullptr);
    CHECK(queue.getElementCount() == 0);
}

TEST_CASE("blocking queue: auto open close")
{
    TestQueue queue(5, &sizeTest);

    // should be closed
    CHECK_FALSE(queue.isOpen());

    {
        TestQueue::AutoOpenClose closer(queue);

        // should be opened
        CHECK(queue.isOpen());
    }

    // should be closed
    CHECK_FALSE(queue.isOpen());
}
