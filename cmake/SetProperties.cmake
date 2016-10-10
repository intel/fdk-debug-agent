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


set(DBGA_VERSION "0.0.0.0" CACHE STRING "Version of the DBGA build")

set(REGEX "([0-9]+).([0-9]+).([0-9]+).([0-9]+)")

if(NOT DBGA_VERSION MATCHES "${REGEX}")
    message(SEND_ERROR "Could not match DBGA_VERSION=`${DBGA_VERSION}' with: ${REGEX}")
else()
    set(DBGA_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(DBGA_VERSION_MINOR ${CMAKE_MATCH_2})
    set(DBGA_VERSION_PATCH ${CMAKE_MATCH_3})
    set(DBGA_VERSION_TWEAK ${CMAKE_MATCH_4})
endif()

set(DBGA_NAME "Intel FDK Debug Agent")
set(DBGA_DESCRIPTION "Tool for runtime debugging of CAVS dsp firmware.")
