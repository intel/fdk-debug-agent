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
