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
#define CATCH_CONFIG_MAIN
#include "cAVS/Logger.hpp"
#include "catch.hpp"

using namespace debug_agent::cavs;

/* Checking that all Logger::Level are correctly converted to string */
TEST_CASE("Logger::Level to string", "[tostring]")
{
    REQUIRE(Logger::toString(Logger::Level::Critical) == "Critical");
    REQUIRE(Logger::toString(Logger::Level::High) == "High");
    REQUIRE(Logger::toString(Logger::Level::Medium) == "Medium");
    REQUIRE(Logger::toString(Logger::Level::Low) == "Low");
    REQUIRE(Logger::toString(Logger::Level::Verbose) == "Verbose");
}

/* Checking that all Logger::Output are correctly converted to string */
TEST_CASE("Logger::Output to string", "[tostring]")
{
    REQUIRE(Logger::toString(Logger::Output::Sram) == "SRAM");
    REQUIRE(Logger::toString(Logger::Output::Pti) == "PTI");
}

/* Checking that all Logger::Level are correctly created from string */
TEST_CASE("Logger::Level from string", "[fromstring]")
{
    REQUIRE(Logger::levelFromString("Critical") == Logger::Level::Critical);
    REQUIRE(Logger::levelFromString("High") == Logger::Level::High);
    REQUIRE(Logger::levelFromString("Medium") == Logger::Level::Medium);
    REQUIRE(Logger::levelFromString("Low") == Logger::Level::Low);
    REQUIRE(Logger::levelFromString("Verbose") == Logger::Level::Verbose);
}

/* Checking that all Logger::Output are correctly created from string */
TEST_CASE("Logger::Output from string", "[fromstring]")
{
    REQUIRE(Logger::outputFromString("SRAM") == Logger::Output::Sram);
    REQUIRE(Logger::outputFromString("PTI") == Logger::Output::Pti);
}

/* Checking that invalid string representation of Logger::Level raises an exception */
TEST_CASE("Failing Logger::Level from string", "[fromstring]")
{
    REQUIRE_THROWS_AS(Logger::levelFromString("something"), Logger::Exception);
}

/* Checking that invalid string representation of Logger::Output raises an exception */
TEST_CASE("Failing Logger::Output from string", "[fromstring]")
{
    REQUIRE_THROWS_AS(Logger::outputFromString("something"), Logger::Exception);
}
