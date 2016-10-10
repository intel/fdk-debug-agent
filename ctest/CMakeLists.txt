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


# Ctest configuration variables must be set BEFORE include(Ctest)

# Check process children and give detail for each leak
set(MEMORYCHECK_COMMAND_OPTIONS
    "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes --leak-check=full")

# As dash is not used to submit results, there is no way to see valgrind result.
# Force it to log to stderr and fail in case of leak or error.
set(MEMORYCHECK_COMMAND_OPTIONS
    "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=255 --log-fd=2")

# Enable tests, coverage, memcheck, ...
# See http://www.cmake.org/Wiki/CMake/Testing_With_CTest#Dashboard_Preparation
include(CTest)

# Convenience method to add tests using "catch" for which we'll set the
# reporter and/or output file
set(catch_driver "${CMAKE_CURRENT_LIST_DIR}/catch_driver.py")
function(add_catch_test executable)
    set(COMMAND python "${catch_driver}" "$<TARGET_FILE:${executable}>")
    if (NOT ENABLE_ADMIN_STEPS)
        set(COMMAND ${COMMAND} "exclude:[admin_rights]")
    endif()
    add_test(
        NAME ${executable}
        COMMAND ${COMMAND})
endfunction()

# Provides a PROJECT_PATH macro that holds the absolute project path, useful to locate test
# data files relatively to the project path
function(provideProjectPathMacro PROJECT_NAME PROJECT_PATH)
    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_DEFINITIONS
        PROJECT_PATH="${PROJECT_PATH}/")
endfunction()
