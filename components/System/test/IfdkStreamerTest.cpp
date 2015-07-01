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
#include <System/IfdkStreamer.hpp>
#include <TestCommon/TestHelpers.hpp>
#include "catch.hpp"
#include <ostream>
#include <string>
#include <stdexcept>

using namespace debug_agent::system;

class IfdkStreamerTest: public IfdkStreamer
{
    using base = IfdkStreamer;
public:

    IfdkStreamerTest(size_t iterations):
        base(testSystem, testFormat, testVersionMajor, testVersionMinor),
        mIterations(iterations),
        mIteration(0),
        mExpectedStream()
    {
        static const std::string propertyKey("TEST");
        static const std::string propertyValue("TRUE");
        // Add a property
        base::addProperty(propertyKey, propertyValue);

        // Compute expected stream: IFDK header, format specific header, then format specific data
        IfdkStreamHeader ifdkStreamHeader(testSystem,
                                          testFormat,
                                          testVersionMajor,
                                          testVersionMinor);
        ifdkStreamHeader.addProperty(propertyKey, propertyValue);

        mExpectedStream << ifdkStreamHeader;
        mExpectedStream << testFormatHeader;

        for (size_t i = 0; i < iterations; ++i) {

            mExpectedStream << i;
        }
    }

    void streamFormatHeader(std::ostream &os) override
    {
        os << testFormatHeader;
    }

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

    const std::stringstream &getExpectedStream()
    {
        return mExpectedStream;
    }

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

    CHECK_THROWS_MSG(outStream << ifdkStreamer, "End of stream");

    CHECK(outStream.str() == ifdkStreamer.getExpectedStream().str());
}