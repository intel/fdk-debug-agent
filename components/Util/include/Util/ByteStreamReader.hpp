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

#include "Util/ByteStreamCommon.hpp"
#include "Util/MemoryStream.hpp"
#include "Util/Buffer.hpp"
#include "Util/Exception.hpp"
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
    using Exception = util::Exception<ByteStreamReader>;

    struct EOSException : public Exception
    {
        using Exception::Exception;
    };

    /** Ownership is not transferred */
    ByteStreamReader(InputStream &input) : mInput(input) {}
    ByteStreamReader(const ByteStreamReader &) = delete;
    ByteStreamReader &operator=(const ByteStreamReader &) = delete;

    /** Read a "simple type" value.
     *
     * "Simple types" are integral types, they can be serialized
     * using a simple memory copy.
     *
     * @tparam T the type of the value to read
     */
    template <typename T>
    typename std::enable_if_t<IsSimpleSerializableType<T>::value> read(T &value)
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
    typename std::enable_if_t<IsEnumSerializableType<T>::value> read(T &value)
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
    typename std::enable_if_t<IsArraySerializableType<T>::value> read(T &value)
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
    typename std::enable_if_t<IsCompoundSerializableType<T>::value> read(T &value)
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
            T element;
            read(element);
            vector.push_back(element);
        }
    }

private:
    template <typename T>
    void readUsingMemoryCopy(T &value)
    {
        std::size_t elementSize = sizeof(T);

        try {
            auto read = mInput.read(reinterpret_cast<StreamByte *>(&value), elementSize);
            if (read < elementSize) {
                throw EOSException("Read failed: end of stream reached");
            }
        } catch (InputStream::Exception &e) {
            throw Exception("Read failed: " + std::string(e.what()));
        }
    }

    InputStream &mInput;
};

class MemoryByteStreamReader : public ByteStreamReader
{
public:
    MemoryByteStreamReader(const Buffer &buffer)
        : ByteStreamReader(mInput), mBuffer(buffer), mInput(buffer)
    {
    }

    /** @return true if stream is fully consumed, i.e. end of stream is reached */
    bool isEOS() const { return mInput.isEOS(); }

    /** @return the underlying buffer */
    const Buffer &getBuffer() const { return mBuffer; }

    /** @return the current stream pointer index */
    std::size_t getPointerOffset() { return mInput.getPointerOffset(); }

private:
    const Buffer &mBuffer;
    MemoryInputStream mInput;
};
}
}
