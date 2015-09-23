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

#include "Util/Buffer.hpp"

namespace debug_agent
{
namespace util
{

/** This buffer maps a typed instance
 * For instance:
 *
 * struct MyStruct {
 *     int member;
 * }
 *
 * TypedBuffer<MyStruct> buffer();
 *
 * // underlying type members can be accessed using -> operator
 * buffer->member = 2;
 *
 * //another way consists in getting a reference to the underlying type using getContent() method
 * MyStruct &struct = buffer.getContent();
 * struct.member = 2;
 *
 * @tparam T the underlying buffer type
 */
template <typename T>
class TypedBuffer : public Buffer
{
public:
    explicit TypedBuffer() : Buffer(sizeof(T)) {}

    /**
     * @param[in] value The value used for buffer initialization
     * @param[in] size The size of the buffer, shall be >= sizeof(T).
     *                  By default size = sizeof(T) */
    explicit TypedBuffer(const T &value) : Buffer(sizeof(T))
    {
        getContent() = value;
    }

    T &getContent()
    {
        return *reinterpret_cast<T *>(data());
    }

    T* operator->()
    {
        return &getContent();
    }
};

}
}
