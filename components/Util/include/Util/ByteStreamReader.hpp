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
#include <stdexcept>
#include <cassert>
#include <inttypes.h>

namespace debug_agent
{
namespace util
{

 /* Read values from a binary stream */
class ByteStreamReader
{
public:
    class Exception : public std::logic_error
    {
    public:
        explicit Exception(const std::string& what)
            : std::logic_error(what)
        {}
    };

    /** @param vector: the input buffer */
    ByteStreamReader(const std::vector<uint8_t> &vector) : mIndex(0), mBuffer(vector) {}

    /** Read a value of type supplied as template parameter
     * @tparam T the type of the value to read
     * @throw ByteStreamReader::Exception if the end of stream is reached
     */
    template <typename T>
    void read(T &value) {
        std::size_t elementSize = sizeof(T);
        if (mIndex + elementSize > mBuffer.size()) {
            throw Exception("Read failed: end of stream reached");
        }
        T *valuePtr = reinterpret_cast<T*>(&mBuffer[mIndex]);
        value = *valuePtr;
        mIndex += elementSize;
    }

    /* Read a vector of type supplied as template parameter
     * The vector size is firstly read from the stream (The type of this size is supplied
     * as template parameter).
     * Then the vector elements are read.
     * @tparam SizeType the type of vector size read from the stream
     * @tparam T the type of the vector elements
     * @throw ByteStreamReader::Exception if the end of stream is reached
     */
    template <typename SizeType, typename T>
    void readVector(std::vector<T> &vector) {
        SizeType size;
        /* Reading the size */
        read(size);

        /* Resizing the vector */
        vector.resize(size);

        /* Reading each element */
        for (auto &element : vector) {
            read(element);
        }
    }

    /* Read a vector of type supplied as template parameter. The supplied type should implements
     * the following method, which is called on all vector elements in order to recurse
     * the serialization:
     *
     * void fromStream(ByteStreamReader &reader);
     *
     * @tparam SizeType the type of vector size read from the stream
     * @tparam T the type of the vector elements, which implements the fromStream() method.
     * @throw ByteStreamReader::Exception if the end of stream is reached
     */
    template <typename SizeType, typename T>
    void readVectorAndRecurse(std::vector<T> &vector) {
        SizeType size;
        /* Reading the size */
        read(size);

        /* Resizing the vector */
        vector.resize(size);

        /* Calling recursively fromStream() on each element */
        for (auto &element : vector) {
            element.fromStream(*this);
        }
    }

private:
    std::size_t mIndex;
    std::vector<uint8_t> mBuffer;
};

}
}
