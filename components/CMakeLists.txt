# Copyright (c) 2016, Intel Corporation
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


if (UNIX)
    # unused-parameter errors in Release mode stem from parameters being only
    # used in assert() calls, which are disabled in that mode.
    set_directory_properties(PROPERTIES
        COMPILE_OPTIONS "-Wall;$<$<CONFIG:Release>:-Wno-unused-parameter>;-Wextra")
endif()

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    DBGA_BUILD_CONFIGURATION=\"$<CONFIG>\")

# Add DBGA_VERSION_* compilation definitions
foreach(version_item DBGA_VERSION_MAJOR DBGA_VERSION_MINOR DBGA_VERSION_PATCH DBGA_VERSION_TWEAK)
    set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
        ${version_item}=\"${${version_item}}\")
endforeach()

# Adding components in the right order (less dependent first)
foreach(dir Util IfdkObjects TestCommon Tlv Rest System cAVS ParameterSerializer Core Main)
    add_subdirectory("${dir}")
endforeach()

if (NOT ${EXCLUDE_TEST_IN_ALL_TARGET})
    add_subdirectory("FunctionalTests")
endif (NOT ${EXCLUDE_TEST_IN_ALL_TARGET})

# Windows only components
if (WIN32)
    add_subdirectory("LogGrabber")
endif()

