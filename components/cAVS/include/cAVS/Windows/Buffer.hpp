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

#include <vector>
#include <memory>
#include <assert.h>
#include <algorithm>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Simple buffer class, used for windows ioctl calls */
class Buffer
{
public:
    /** @param[in] size the buffer byte size */
    explicit Buffer(std::size_t size) : mBuffer(size)
    {
        /* Setting the initial memory content using a non 0 value. This is required by
         * the mocked up driver, because it checks that buffers are the expected ones,
         * and sometime the user doesn't set the full buffer content. Therefore without this
         * memory set, areas not set by the user are unpredictables.
         *
         * This may lead to performance issue. If it is the case, consider activating
         * this memory set only in mocking case. */
        memset(getPtr(), 0xFF, getSize());
    }

    explicit Buffer(const Buffer &other)
    {
        mBuffer = other.mBuffer;
    }

    explicit Buffer(const std::vector<char> &source):
        Buffer(source.size())
    {
        static_assert(sizeof(char) == sizeof(uint8_t),
            "Code designed for sizeof(char) == sizeof(uint8_t)");

        const uint8_t * sourceData = reinterpret_cast<const uint8_t *>(source.data());
        std::copy(sourceData, sourceData + source.size(), mBuffer.data());
    }

    virtual ~Buffer() {}

    /**
     * @param[in] size the new size of the buffer in bytes.
     */
    void resize(size_t size)
    {
        mBuffer.resize(size);
    }

    void *getPtr()
    {
        return mBuffer.data();
    }

    const void *getPtr() const
    {
        return mBuffer.data();
    }

    std::size_t getSize() const
    {
        return mBuffer.size();
    }

    bool operator == (const Buffer& other) const
    {
        return mBuffer == other.mBuffer;
    }

    bool operator != (const Buffer& other) const
    {
        return !(*this == other);
    }

    void copyFrom(const Buffer &other)
    {
        /* Copying a buffer with a different size is forbidden */
        assert(getSize() == other.getSize());

        mBuffer = other.mBuffer;
    }

private:

    Buffer &operator=(const Buffer &other) = delete;

    std::vector<uint8_t> mBuffer;
};

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
 * The constructor takes an optional size parameter: this size can be used to allocate a memory
 * area greater than the underlying structure, this is often useful for ioctl structures.
 * For instance this one:
 *
 * typedef struct _Intc_Set_Parameter
 * {
 *    NTSTATUS Status;
 *    UCHAR Data[1];
 * } Intc_Set_Parameter;
 *
 * The member 'Data' of the struct Intc_Set_Parameter matches the type "UCHAR[1]", but in reality
 * the signification of this type is "char array with an arbitrary size".So if the real 'Data'
 * member size is 128:
 *
 * TypedBuffer<_Intc_Set_Parameter> buffer(sizeof(_Intc_Set_Parameter) + (128-1) * sizeof(UCHAR));
 * buffer->Data[120] = 2; // index valid in range 0..127
 *
 * @tparam T the underlying buffer type
 */
template <typename T>
class TypedBuffer : public Buffer
{
public:
    /** @param[in] size The size of the buffer, shall be >= sizeof(T).
     *                  By default size = sizeof(T) */
    explicit TypedBuffer(std::size_t size = sizeof(T)) : Buffer(size)
    {
        assert(size >= sizeof(T));
    }

    /**
     * @param[in] value The value used for buffer initialization
     * @param[in] size The size of the buffer, shall be >= sizeof(T).
     *                  By default size = sizeof(T) */
    explicit TypedBuffer(const T &value, std::size_t size = sizeof(T)) : Buffer(size)
    {
        assert(size >= sizeof(T));
        getContent() = value;
    }

    T &getContent()
    {
        return *static_cast<T *>(getPtr());
    }

    T* operator->()
    {
        return &getContent();
    }
};

}
}
}
