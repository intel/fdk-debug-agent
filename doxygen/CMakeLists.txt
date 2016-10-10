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


if(DOXYGEN)

    find_package(Doxygen REQUIRED)

    if(NOT DOXYGEN_DOT_FOUND)
        message(SEND_ERROR "The 'dot' utility was  not found")
    endif()

    set(DOXYGEN_SEARCHENGINE YES)
    set(DOT_IMAGE_FORMAT svg)
    if(WIN32)
        # On windows, generate a CHM file
        set(DOXYGEN_GENERATE_HTMLHELP YES)
        # The html search engin must be disabled when generating a CHM file
        set(DOXYGEN_SEARCHENGINE NO)
        set(DOXYGEN_HHC_LOCATION "C:/Program Files (x86)/HTML Help Workshop/hhc.exe")
        # There seems to be an issue with plantuml-generated SVG diagrams on the CHM viewer. Switch
        # to PNG instead.
        set(DOT_IMAGE_FORMAT png)
    endif()

    # Configures path information and some variables defined by `find_package(Doxygen)`
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
       @ONLY)
    add_custom_target(doxygen
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM)
endif()
