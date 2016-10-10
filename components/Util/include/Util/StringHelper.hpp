/*
 * Copyright (c) 2015-2016, Intel Corporation
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

#include <sstream>
#include <string>
#include <cassert>
#include <cctype>

namespace debug_agent
{
namespace util
{

class StringHelper final
{
public:
    /** This method returns a std::string from a byte buffer
     *
     *  @tparam ArrayElementType the array element type, its size must be one byte.
     *                          For instance : int8_t, uint8_t, char, unsigned char ...
     */

    template <typename ArrayElementType>
    static std::string getStringFromFixedSizeArray(ArrayElementType *buffer, std::size_t size)
    {
        static_assert(sizeof(ArrayElementType) == 1, "Size of ArrayElementType must be one");

        std::stringstream stream;
        for (std::size_t i = 0; i < size && buffer[i] != 0; i++) {
            stream << static_cast<char>(buffer[i]);
        }
        return stream.str();
    }

    /** This method fills a fixed-size array from a string
     *
     * If the supplied string is bigger than the buffer size, an assertion is thrown.
     *
     *  @tparam ArrayElementType the array element type, its size must be one byte.
     *                          For instance : int8_t, uint8_t, char, unsigned char ...
     */
    template <typename ArrayElementType>
    static void setStringToFixedSizeArray(ArrayElementType *buffer, std::size_t size,
                                          const std::string &str)
    {
        static_assert(sizeof(ArrayElementType) == 1, "Size of ArrayElementType must be one");

        assert(str.size() <= size);
        std::size_t copySize = std::min(size, str.size());

        for (std::size_t i = 0; i < copySize; i++) {
            buffer[i] = str[i];
        }
        for (std::size_t i = copySize; i < size; i++) {
            buffer[i] = 0;
        }
    }

    /* Trim string whitespaces */
    static std::string trim(const std::string &str)
    {
        auto beginIt = str.begin();
        while (beginIt != str.end() && std::isspace(*beginIt)) {
            beginIt++;
        }

        auto endIt = str.rbegin();
        while (endIt != str.rend() && std::isspace(*endIt)) {
            endIt++;
        }

        std::size_t offset = beginIt - str.begin();
        std::size_t endOffset = str.length() - (endIt - str.rbegin());
        std::size_t length = 0;
        if (endOffset > offset) {
            length = endOffset - offset;
        }

        return str.substr(offset, length);
    }

    static bool startWith(const std::string &str, const std::string &prefix)
    {
        return str.compare(0, prefix.size(), prefix) == 0;
    }

private:
    StringHelper();
};
}
}
