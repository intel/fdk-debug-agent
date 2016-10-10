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
