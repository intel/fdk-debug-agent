/*
 * Copyright (c) 2015, Intel Corporation
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
#include <vector>
#include <limits>
#include <cassert>
#include <inttypes.h>
#include <type_traits>

namespace debug_agent
{
namespace util
{

/* Write types to a binary stream */
class ByteStreamWriter
{
public:
    using Exception = util::Exception<ByteStreamWriter>;

    ByteStreamWriter(OutputStream &os) : mOutput(os) {}
    ByteStreamWriter(const ByteStreamWriter &) = delete;
    ByteStreamWriter &operator=(const ByteStreamWriter &) = delete;

    /** Write a "simple type" value.
     *
     * "Simple types" are integral types, they can be serialized
     * using a simple memory copy.
     *
     * @tparam T the type of the value to write
     */
    template <typename T>
    typename std::enable_if_t<IsSimpleSerializableType<T>::value> write(const T &value)
    {
        writeUsingMemoryCopy(value);
    }

    /** Write a "enum type" value.
     *
     * "Enum types" are serialized using the type EnumEncodingType
     *
     * @tparam T the type of the value to write
     */
    template <typename T>
    typename std::enable_if_t<IsEnumSerializableType<T>::value> write(const T &value)
    {
        static_assert(sizeof(T) <= sizeof(EnumEncodingType), "Enum type size is too big");

        EnumEncodingType encoding = static_cast<EnumEncodingType>(value);
        writeUsingMemoryCopy(encoding);
    }

    /** Read an array value.
     *
     * C arrays and std::arrays are serialized by iteratively serializing each
     * of their items.
     *
     * @tparam T the type of the value to read
     */
    template <typename T>
    typename std::enable_if_t<IsArraySerializableType<T>::value> write(const T &value)
    {
        for (const auto &item : value) {
            write(item);
        }
    }

    /** Write a "compound type" value.
     *
     * "Compound types" implement the toStream() method in order to serialize contained members.
     *
     * @tparam T the type of the value to write, shall implement the toStream() implicit interface.
     */
    template <typename T>
    typename std::enable_if_t<IsCompoundSerializableType<T>::value> write(const T &value)
    {
        value.toStream(*this);
    }

    /** Write a raw buffer. Its size is not written in the stream */
    void writeRawBuffer(const util::Buffer &buffer)
    {
        try {
            mOutput.write(buffer.data(), buffer.size());
        } catch (InputStream::Exception &e) {
            throw Exception("Write raw buffer failed: " + std::string(e.what()));
        }
    }

    /* Write a vector of type supplied as template parameter, the supplied type could be either
     * simple or composite.
     *
     * The vector size is firstly written to the stream (The type of this size is supplied
     * as template parameter).
     * Then the vector elements are written.
     * @tparam SizeType the type of vector size written to the stream
     * @tparam T the type of the vector elements
     */
    template <typename SizeType, typename T>
    void writeVector(const std::vector<T> &vector)
    {

        /* Checking that size can be encoded with the supplied type */
        assert(vector.size() <= std::numeric_limits<SizeType>::max());

        SizeType size = static_cast<SizeType>(vector.size());
        /* Writing the size */
        write(size);

        /* Writing the elements*/
        for (auto &element : vector) {
            write(element);
        }
    }

private:
    template <typename T>
    void writeUsingMemoryCopy(const T &value)
    {
        std::size_t elementSize = sizeof(T);

        try {
            mOutput.write(reinterpret_cast<const StreamByte *>(&value), elementSize);
        } catch (InputStream::Exception &e) {
            throw Exception("Write failed: " + std::string(e.what()));
        }
    }

    OutputStream &mOutput;
};

class MemoryByteStreamWriter : public ByteStreamWriter
{
public:
    MemoryByteStreamWriter() : ByteStreamWriter(mOutput), mOutput(mBuffer) {}

    /** @return the underlying buffer */
    const Buffer &getBuffer() const { return mBuffer; }

private:
    Buffer mBuffer;
    MemoryOutputStream mOutput;
};
}
}
