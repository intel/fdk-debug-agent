################################################################################
#                              INTEL CONFIDENTIAL
#   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
#   The source code contained  or  described herein and all documents related to
#   the source code ("Material") are owned by Intel Corporation or its suppliers
#   or licensors.  Title to the  Material remains with  Intel Corporation or its
#   suppliers and licensors. The Material contains trade secrets and proprietary
#   and  confidential  information of  Intel or its suppliers and licensors. The
#   Material  is  protected  by  worldwide  copyright  and trade secret laws and
#   treaty  provisions. No part of the Material may be used, copied, reproduced,
#   modified, published, uploaded, posted, transmitted, distributed or disclosed
#   in any way without Intel's prior express written permission.
#   No license  under any  patent, copyright, trade secret or other intellectual
#   property right is granted to or conferred upon you by disclosure or delivery
#   of the Materials,  either expressly, by implication, inducement, estoppel or
#   otherwise.  Any  license  under  such  intellectual property  rights must be
#   express and approved by Intel in writing.
#
################################################################################

# cmake configuration file of the "Util" component

set(SRCS
    src/Uuid.cpp)

set(INCS
    include/Util/AssertAlways.hpp
    include/Util/BlockingQueue.hpp
    include/Util/Buffer.hpp
    include/Util/ByteStreamCommon.hpp
    include/Util/ByteStreamReader.hpp
    include/Util/ByteStreamWriter.hpp
    include/Util/EnumHelper.hpp
    include/Util/Exception.hpp
    include/Util/FileHelper.hpp
    include/Util/Locker.hpp
    include/Util/PointerHelper.hpp
    include/Util/RingBufferReader.hpp
    include/Util/Stream.hpp
    include/Util/StringHelper.hpp
    include/Util/StructureChangeTracking.hpp
    include/Util/TypedBuffer.hpp
    include/Util/Cxx17Backports.hpp
    include/Util/Uuid.hpp
    include/Util/WrappedRaw.hpp
    include/Util/convert.hpp
)

add_library(Util STATIC ${SRCS} ${INCS})
set_common_settings(Util)

# exporting include directory
target_include_directories(Util PUBLIC "include")

# test
add_subdirectory("test")
