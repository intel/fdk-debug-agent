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
#pragma once

#include "Util/Buffer.hpp"
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

inline static util::Buffer readAsBytes(const std::string &fileName)
{
    std::ifstream file(fileName);
    if (!file) { /* Using stream bool operator */
        throw Exception("Unable to open file: " + fileName);
    }

    Buffer content{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};

    if (file.bad()) {
        throw Exception("Error while reading file: " + fileName);
    }

    return content;
}

/** Create a file from string */
inline static void writeFromBytes(const std::string &fileName, const util::Buffer &content)
{
    std::ofstream file(fileName, std::ios::binary);
    if (!file) { /* Using stream bool operator */
        throw Exception("Unable to create file: " + fileName);
    }

    file.write(reinterpret_cast<const char *>(content.data()), content.size());

    if (file.bad()) {
        throw Exception("Error while writing file: " + fileName);
    }
}

/** Read file content as string */
inline static std::string readAsString(const std::string &fileName)
{
    auto buffer = readAsBytes(fileName);
    return std::string(buffer.begin(), buffer.end());
}

/** Create a file from string */
inline static void writeFromString(const std::string &fileName, const std::string &content)
{
    Buffer buffer(content.begin(), content.end());
    writeFromBytes(fileName, buffer);
}

/** Remove a file */
inline static void remove(const std::string &fileName)
{
    if (std::remove(fileName.c_str()) != 0) {
        throw Exception("Cannot remove file: " + fileName);
    }
}
}
}
}
