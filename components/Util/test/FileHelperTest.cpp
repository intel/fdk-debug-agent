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
#include "Util/FileHelper.hpp"
#include <catch.hpp>

using namespace debug_agent::util;

TEST_CASE("file_helper: all methods")
{
    static const std::string content{"This is a content\nIncredible, isn't it?"};
    static const std::string fileName{"test.txt"};

    /* Writing */
    CHECK_NOTHROW(file_helper::writeFromString(fileName, content));

    /* Reading */
    std::string readContent;
    CHECK_NOTHROW(readContent = file_helper::readAsString(fileName));

    /* Checking read content*/
    CHECK(readContent == content);

    /* Deleting */
    CHECK_NOTHROW(file_helper::remove(fileName));

    /* Writing failure */
    CHECK_THROWS_AS(file_helper::writeFromString("unexisting_dir/" + fileName, content),
                    file_helper::Exception);

    /* Reading failure (file no more exists) */
    CHECK_THROWS_AS(file_helper::readAsString(fileName), file_helper::Exception);

    /* File deletion failure */
    CHECK_THROWS_AS(file_helper::remove(fileName), file_helper::Exception);
}
