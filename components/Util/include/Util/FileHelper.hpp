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

#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdio>

namespace debug_agent
{
namespace util
{
namespace file_helper
{
struct Exception : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/** Read file content as string */
static std::string readAsString(const std::string &fileName)
{
    std::ifstream file(fileName);
    if (!file) { /* Using stream bool operator */
        throw Exception("Unable to open file: " + fileName);
    }

    std::string content{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    if (file.bad()) {
        throw Exception("Error while reading file: " + fileName);
    }

    return content;
}

/** Create a file from string */
static void writeFromString(const std::string &fileName, const std::string &content)
{
    std::ofstream file(fileName);
    if (!file) { /* Using stream bool operator */
        throw Exception("Unable to create file: " + fileName);
    }

    file << content;

    if (file.bad()) {
        throw Exception("Error while writing file: " + fileName);
    }
}

/** Remove a file */
static void remove(const std::string &fileName)
{
    if (std::remove(fileName.c_str()) != 0) {
        throw Exception("Cannot remove file: " + fileName);
    }
}
}
}
}
