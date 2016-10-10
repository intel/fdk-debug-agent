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


# cmake configuration file of the "Main" component

# Main executable
set(SRCS
    src/Application.cpp
    src/Main.cpp)

set(INCS
    include/Main/Application.hpp)

if (WIN32)
    # use to handle versioning
    include("CMakeListsWindows.txt")
endif (WIN32)

add_dbga_executable(Main ${SRCS} ${INCS})
set_common_settings(Main)

if (WIN32)
    # executable will ask administrator privileges
    set_target_properties(Main PROPERTIES LINK_FLAGS
        "/level='requireAdministrator' /uiAccess='false'")
endif (WIN32)

target_compile_definitions(Main PRIVATE
    WINRC_EXE
    WINRC_FILENAME="$<TARGET_FILE_NAME:Main>")

target_include_directories(Main PRIVATE "include")

# Setting executable name to DebugAgent
set_target_properties(Main PROPERTIES OUTPUT_NAME "DebugAgent")

# Binding with Poco
link_poco(Main)

# Needed components
target_link_libraries(Main Core Util)

# Installation step
install(TARGETS Main RUNTIME DESTINATION bin)
