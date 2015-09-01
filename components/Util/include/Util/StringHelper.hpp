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

#include <sstream>
#include <string>

namespace debug_agent
{
namespace util
{

class StringHelper final
{
public:

    /** This method returns a std::string from a byte buffer
     *
     *  @tparam ArrayElementType the array element type, its size must be one byte.
     *                          For instance : int8_t, uint8_t, char, unsigned char ...
     */

    template<typename ArrayElementType>
    static std::string getStringFromFixedSizeArray(ArrayElementType *buffer, std::size_t size)
    {
        static_assert(sizeof(ArrayElementType) == 1, "Size of ArrayElementType must be one");

        std::stringstream stream;
        for (std::size_t i = 0; i < size && buffer[i] != 0; i++) {
            stream << static_cast<char>(buffer[i]);
        }
        return stream.str();
    }

private:
    StringHelper();
};

}
}
