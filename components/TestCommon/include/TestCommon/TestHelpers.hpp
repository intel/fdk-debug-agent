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

/**
 * This macro is an extension of CATCH framework
 * Check:
 * - if a specific exception is thrown
 * - if the exception message matches the expected one */
#define CHECK_THROWS_AS_MSG(expr, eType, msg) \
    try {                                     \
        { expr; }                             \
        INFO("Exception should be thrown");   \
        CHECK(false);                         \
    }                                         \
    catch (eType &e) {                        \
        CHECK(std::string(e.what()) == msg);  \
    }                                         \
    catch (...) {                             \
        INFO("Unexpected exception");         \
        CHECK(false);                         \
    }

/**
 * This macro is an extension of CATCH framework
 * Check:
 * - if a specific exception is thrown
 * - if the exception message matches the expected one */
#define REQUIRE_THROWS_AS_MSG(expr, eType, msg) \
    try {                                       \
        { expr; }                               \
        INFO("Exception should be thrown");     \
        REQUIRE(false);                         \
    }                                           \
    catch (eType &e) {                          \
        REQUIRE(std::string(e.what()) == msg);  \
    }                                           \
    catch (...) {                               \
        INFO("Unexpected exception");           \
        REQUIRE(false);                         \
    }
