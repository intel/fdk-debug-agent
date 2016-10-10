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
#include <System/IfdkStreamer.hpp>
#include <TestCommon/TestHelpers.hpp>
#include "catch.hpp"
#include <ostream>
#include <string>
#include <stdexcept>

using namespace debug_agent::system;

class IfdkStreamerTest : public IfdkStreamer
{
    using base = IfdkStreamer;

public:
    IfdkStreamerTest(size_t iterations)
        : base(testSystem, testFormat, testVersionMajor, testVersionMinor), mIterations(iterations),
          mIteration(0), mExpectedStream()
    {
        static const std::string propertyKey("TEST");
        static const std::string propertyValue("TRUE");
        // Add a property
        base::addProperty(propertyKey, propertyValue);

        // Compute expected stream: IFDK header, format specific header, then format specific data
        IfdkStreamHeader ifdkStreamHeader(testSystem, testFormat, testVersionMajor,
                                          testVersionMinor);
        ifdkStreamHeader.addProperty(propertyKey, propertyValue);

        mExpectedStream << ifdkStreamHeader;
        mExpectedStream << testFormatHeader;

        for (size_t i = 0; i < iterations; ++i) {

            mExpectedStream << i;
        }
    }

    void streamFormatHeader(std::ostream &os) override { os << testFormatHeader; }

    bool streamNextFormatData(std::ostream &os) override
    {
        if (mIteration < mIterations) {

            // Test streams is just mIteration streamed out (formatted)
            os << mIteration++;
            return true;
        } else {

            return false;
        }
    }

    const std::stringstream &getExpectedStream() { return mExpectedStream; }

    const size_t mIterations;
    size_t mIteration;
    std::stringstream mExpectedStream;

    static const std::string testFormatHeader;
    static const std::string testSystem;
    static const std::string testFormat;
    static const unsigned int testVersionMajor;
    static const unsigned int testVersionMinor;
};

const std::string IfdkStreamerTest::testFormatHeader("The header of this amazing format!");
const std::string IfdkStreamerTest::testSystem("TestSystem");
const std::string IfdkStreamerTest::testFormat("TestFormat");
const unsigned int IfdkStreamerTest::testVersionMajor = 1;
const unsigned int IfdkStreamerTest::testVersionMinor = 9;

TEST_CASE("Test IFDK stream", "[stream]")
{
    IfdkStreamerTest ifdkStreamer(50);
    std::stringstream outStream;

    CHECK_NOTHROW(outStream << ifdkStreamer);

    CHECK(outStream.str() == ifdkStreamer.getExpectedStream().str());
}