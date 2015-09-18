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
std::size_t sizeTest(const Test& test)
{
    return test.mSize;
}

using TestPtr = std::unique_ptr<Test>;
using TestQueue = BlockingQueue<Test>;

std::unique_ptr<Test> makeTest(std::size_t size)
{
    return std::unique_ptr<Test>(new Test(size));
}

TEST_CASE("blocking queue: simple monothread usage")
{
    TestQueue queue(5, &sizeTest);

    CHECK(queue.add(makeTest(5)));
    queue.close();
    CHECK_FALSE(queue.add(makeTest(2)));

    TestPtr element(queue.remove());
    CHECK(element != nullptr);
    CHECK(element->mSize == 5);

    element = std::move(queue.remove());
    CHECK(element == nullptr);
}

TEST_CASE("blocking queue: dropping")
{
    TestQueue queue(10, &sizeTest); /* Maximum memory size: 10 bytes */

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

    CHECK(queue.add(makeTest(2)));
    CHECK(queue.add(makeTest(5)));

    queue.clear();
    CHECK(queue.getElementCount() == 0);
}


TEST_CASE("blocking queue: multithread closing")
{
    TestQueue queue(10, &sizeTest);

    /* Closing in another thread */
    std::future<void> result(std::async(std::launch::async, [&]() {
        queue.close();
    }));

    TestPtr element(queue.remove());
    CHECK(element == nullptr);

    result.get();
}

TEST_CASE("blocking queue: multi theading usage")
{
    TestQueue queue(10, &sizeTest);

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
    element = std::move(queue.remove());
    CHECK(element != nullptr);
    CHECK(element->mSize == 2);

    element = std::move(queue.remove());
    CHECK(element != nullptr);
    CHECK(element->mSize == 5);

    element = std::move(queue.remove());
    CHECK(element != nullptr);
    CHECK(element->mSize == 3);

    /* Closed: last element is null */
    element = std::move(queue.remove());
    CHECK(element == nullptr);

    /* Checking that adding element is successful */
    CHECK(futureResult.get());
}


