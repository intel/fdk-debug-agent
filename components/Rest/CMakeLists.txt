# Copyright (c) 2015, Intel Corporation
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


# cmake configuration file of the "Rest" component

# library
set(LIB_SRCS
    src/Server.cpp
    src/Dispatcher.cpp
    src/ServerRequestHandling.cpp
    src/Resource.cpp)

set(LIB_INCS
    include/Rest/Server.hpp
    include/Rest/Request.hpp
    include/Rest/Response.hpp
    include/Rest/StreamResponse.hpp
    include/Rest/CustomResponse.hpp
    include/Rest/Dispatcher.hpp
    include/Rest/Resource.hpp
    include/Rest/ErrorHandler.hpp
    src/ServerRequestHandling.hpp) # private header

add_library(Rest STATIC ${LIB_SRCS} ${LIB_INCS})
set_common_settings(Rest)

# including private headers in src directory
target_include_directories(Rest PRIVATE "src")

# exporting "include" directory
target_include_directories(Rest PUBLIC "include")

# external libraries
link_poco(Rest)

# adding test
add_subdirectory("test")

# Needed components
target_link_libraries(Rest Util)

