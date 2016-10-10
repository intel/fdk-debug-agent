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

#include "Util/MemoryStream.hpp"
#include "Util/AssertAlways.hpp"
#include <catch.hpp>

using namespace debug_agent::util;

bool startsWith(const Buffer &buffer, const Buffer &prefix)
{
    ASSERT_ALWAYS(buffer.size() >= prefix.size());
    return std::equal(buffer.begin(), buffer.begin() + prefix.size(), prefix.begin());
}

SCENARIO("MemoryInputStream")
{
    Buffer output(10);
    uint8_t *outputBuffer = &output[0];

    GIVEN ("An empty source buffer") {
        Buffer source;
        WHEN ("Reading from it") {
            MemoryInputStream is(source);
            auto read = is.read(outputBuffer, output.size());
            THEN ("No byte is read") {
                CHECK(read == 0);
            }
        }
    }
    GIVEN ("A source buffer with 4 elements") {
        Buffer source{0, 1, 2, 3};
        WHEN ("Reading whole source buffer (4 elements)") {
            MemoryInputStream is(source);
            auto read = is.read(outputBuffer, 4);
            THEN ("4 bytes are read") {
                CHECK(read == 4);
                CHECK(startsWith(output, source));
                WHEN ("Reading again") {
                    read = is.read(outputBuffer, output.size());
                    THEN ("0 byte is read") {
                        CHECK(read == 0);
                    }
                }
            }
        }
        WHEN ("Reading more that source buffer size (10 elements)") {
            MemoryInputStream is(source);
            auto read = is.read(outputBuffer, output.size());
            THEN ("4 bytes are read") {
                CHECK(read == 4);
            }
        }
        WHEN ("Reading less that source buffer size : 2 bytes") {
            MemoryInputStream is(source);
            auto read = is.read(outputBuffer, 2);
            THEN ("2 bytes are read") {
                CHECK(read == 2);
                CHECK(startsWith(output, {0, 1}));
                WHEN ("Read 1 byte more") {
                    read = is.read(outputBuffer, 1);
                    THEN ("1 byte is read") {
                        CHECK(read == 1);
                        CHECK(startsWith(output, {2}));
                        WHEN ("Read 2 bytes more") {
                            read = is.read(outputBuffer, 2);
                            THEN ("1 byte is read") {
                                CHECK(read == 1);
                                CHECK(startsWith(output, {3}));
                            }
                        }
                    }
                }
            }
        }
    }
}

SCENARIO("MemoryOutputStream")
{
    Buffer output;
    MemoryOutputStream os(output);

    WHEN ("Writing 0 byte") {
        uint8_t source[]{0};
        os.write(source, 0);
        THEN ("Output buffer is empty") {
            CHECK(output.empty());
        }
    }
    WHEN ("Writing source buffer (4 elements)") {
        uint8_t source[]{0, 1, 2, 3};
        os.write(source, sizeof(source));
        THEN ("Output buffer size is 4") {
            CHECK((output == Buffer{0, 1, 2, 3}));
            WHEN ("Writing two bytes more") {
                uint8_t source2[]{4, 5};
                os.write(source2, sizeof(source2));
                THEN ("Output buffer size is 6") {
                    CHECK((output == Buffer{0, 1, 2, 3, 4, 5}));
                }
            }
        }
    }
}
