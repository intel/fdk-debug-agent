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
#include <vector>
#include <limits>
#include <cassert>
#include <inttypes.h>

/* The visual studio toolchain define the "max" macro, which makes fail the call to
 * std::numeric_limits<SizeType>::max(). So undefining it.
 */
#ifdef max
#undef max
#endif

namespace debug_agent
{
namespace util
{

/* Write types to a binary stream */
class ByteStreamWriter
{
public:
    ByteStreamWriter() {}

    /** Write a value of type supplied as template parameter
     * @tparam T the type of the value to write
     */
    template <typename T>
    void write(const T& value) {
        std::size_t elementSize = sizeof(T);
        const uint8_t *valuePtr = reinterpret_cast<const uint8_t*>(&value);

        /* Write the value as bytes*/
        mBuffer.insert(mBuffer.end(), valuePtr, valuePtr + elementSize);
    }

    /** Write an array of elements
      * @param array the array to write
      * @param count the array size
      * @tparam T the element type
      */
    template <typename T>
    void writeArray(const T *array, std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            write(array[i]);
        }
    }

    /** Write a raw buffer. Its size is not written in the stream */
    void writeRawBuffer(const util::Buffer &buffer)
    {
        mBuffer.insert(mBuffer.end(), buffer.begin(), buffer.end());
    }

    /* Write a vector of type supplied as template parameter
     * The vector size is firstly written to the stream (The type of this size is supplied
     * as template parameter).
     * Then the vector elements are written.
     * @tparam SizeType the type of vector size written to the stream
     * @tparam T the type of the vector elements
     */
    template <typename SizeType, typename T>
    void writeVector(const std::vector<T> &vector) {

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

    /** Write a vector of type supplied as template parameter. The supplied type should implements
     * the following method, which is called on all vector elements in order to recurse
     * the serialization:
     *
     * void toStream(ByteStreamWriter &writer) const;
     *
     * @tparam SizeType the type of vector size written to the stream
     * @tparam T the type of the vector elements, which implements the toStream() method.
     */
    template <typename SizeType, typename T>
    void writeVectorAndRecurse(const std::vector<T> &vector) {

        /* Checking that size can be encoded with the supplied type */
        assert(vector.size() <= std::numeric_limits<SizeType>::max());

        SizeType size = static_cast<SizeType>(vector.size());
        /* Writing the size */
        write(size);

        /* Calling recursively toStream() on each element */
        for (auto &element : vector) {
            element.toStream(*this);
        }
    }

    /** Return the produced buffer */
    const util::Buffer &getBuffer() const
    {
        return mBuffer;
    }

private:
    util::Buffer mBuffer;
};

}
}
