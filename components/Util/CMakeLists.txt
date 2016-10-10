# Copyright (c) 2015-2016, Intel Corporation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# This is the main cmake file of the DebugAgent project


# cmake configuration file of the "Util" component

set(SRCS
    src/Uuid.cpp)

set(INCS
    include/Util/About.hpp
    include/Util/AssertAlways.hpp
    include/Util/BlockingQueue.hpp
    include/Util/Buffer.hpp
    include/Util/ByteStreamCommon.hpp
    include/Util/ByteStreamReader.hpp
    include/Util/ByteStreamWriter.hpp
    include/Util/EnumHelper.hpp
    include/Util/Exception.hpp
    include/Util/FileHelper.hpp
    include/Util/Iterator.hpp
    include/Util/Locker.hpp
    include/Util/MemoryStream.hpp
    include/Util/PointerHelper.hpp
    include/Util/RingBuffer.hpp
    include/Util/RingBufferBase.hpp
    include/Util/RingBufferReader.hpp
    include/Util/RingBufferWriter.hpp
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
