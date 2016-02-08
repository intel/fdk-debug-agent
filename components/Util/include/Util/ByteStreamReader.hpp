/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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

#include "Util/ByteStreamCommon.hpp"
#include "Util/Buffer.hpp"
#include <vector>
#include <stdexcept>
#include <cassert>
#include <inttypes.h>
#include <type_traits>

namespace debug_agent
{
namespace util
{

/* Read values from a binary stream */
class ByteStreamReader
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /** @param vector: the input buffer */
    ByteStreamReader(const util::Buffer &vector) : mIndex(0), mBuffer(vector) {}

    /** Read a "simple type" value.
     *
     * "Simple types" are integral types, they can be serialized
     * using a simple memory copy.
     *
     * @tparam T the type of the value to read
     */
    template <typename T>
    typename std::enable_if<IsSimpleSerializableType<T>::value>::type read(T &value)
    {
        readUsingMemoryCopy(value);
    }

    /** Read a "enum type" value.
     *
     * "Enum types" are serialized using the type EnumEncodingType
     *
     * @tparam T the type of the value to read
     */
    template <typename T>
    typename std::enable_if<IsEnumSerializableType<T>::value>::type read(T &value)
    {
        static_assert(sizeof(T) <= sizeof(EnumEncodingType), "Enum type size is too big");
        EnumEncodingType encoded;
        readUsingMemoryCopy(encoded);
        value = static_cast<T>(encoded);
    }

    /** Read an array value.
     *
     * C arrays and std::arrays are deserialized by iteratively deserializing
     * each of their items.
     *
     * @tparam T the type of the value to read
     */
    template <typename T>
    typename std::enable_if<IsArraySerializableType<T>::value>::type read(T &value)
    {
        for (auto &item : value) {
            read(item);
        }
    }

    /** Read a "compound type" value.
     *
     * "Compound types" implement the fromStream() method in order to serialize contained members.
     *
     * @tparam T the type of the value to read, shall implement the fromStream() implicit
     *           interface.
     */
    template <typename T>
    typename std::enable_if<IsCompoundSerializableType<T>::value>::type read(T &value)
    {
        value.fromStream(*this);
    }

    /* Read a vector of type supplied as template parameter, the supplied type could be either
     * simple or composite.
     *
     * The vector size is firstly read from the stream (The type of this size is supplied
     * as template parameter).
     * Then the vector elements are read.
     * @tparam SizeType the type of vector size read from the stream
     * @tparam T the type of the vector elements
     * @throw ByteStreamReader::Exception if the end of stream is reached
     */
    template <typename SizeType, typename T>
    void readVector(std::vector<T> &vector)
    {
        /* Initializing size because otherwise klocwork believes that an uninitialized value is
         * used, which is a false positive */
        SizeType size = 0;
        /* Reading the size */
        read(size);

        /* Do not resize the vector to the read size: if the size is erroneous, it may make the
         * allocation fail.
         *
         * It's safer to use the "auto-growth" feature of the std::vector. In this way the
         * end of stream will be reached before saturating the memory.
         */

        /* Reading each element */
        for (std::size_t i = 0; i < size; ++i) {
            /* Initializing element with default values because otherwise klocwork believes that
             * uninitialized values are used, which is a false positive */
            T element{};
            read(element);
            vector.push_back(element);
        }
    }

    /** @return true if stream is fully consumed, i.e. end of stream is reached */
    bool isEOS() const { return mIndex == mBuffer.size(); }

    /** @return the underlying buffer */
    const util::Buffer &getBuffer() const { return mBuffer; }

    /** @return the current stream pointer index */
    std::size_t getPointerOffset() { return mIndex; }

private:
    template <typename T>
    void readUsingMemoryCopy(T &value)
    {
        std::size_t elementSize = sizeof(T);
        if (mIndex + elementSize > mBuffer.size()) {
            /* Setting index to buffer size, in this way subsequent calls to isEOS() will return
            * true */
            mIndex = mBuffer.size();
            throw Exception("Read failed: end of stream reached");
        }
        T *valuePtr = reinterpret_cast<T *>(&mBuffer[mIndex]);
        value = *valuePtr;
        mIndex += elementSize;
    }

    std::size_t mIndex;
    util::Buffer mBuffer;
};
}
}
