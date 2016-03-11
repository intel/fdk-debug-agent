################################################################################
#                              INTEL CONFIDENTIAL
#   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
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
function(add_catch_test executable working_dir)
    set(COMMAND python "${catch_driver}" "$<TARGET_FILE:${executable}>")
    if (NOT ENABLE_ADMIN_STEPS)
        set(COMMAND ${COMMAND} "exclude:[admin_rights]")
    endif()
    add_test(
        NAME ${executable}
        COMMAND ${COMMAND}
        WORKING_DIRECTORY ${working_dir})
endfunction()

# Add test executable dependencies using the PATH environment variable
# Note: this is done only for Windows because Linux uses the
# "rpath" feature to find dependent shared libraries.
function(add_test_dependencies executable)
    if(WIN32)
        set(TEST_PATH "$ENV{PATH}")
        foreach(DEP ${ARGN})
            set(TEST_PATH "$<TARGET_FILE_DIR:${DEP}>;${TEST_PATH}")
        endforeach()

        #escaping list delimiter ";"
        string(REPLACE ";" "\\;" TEST_PATH "${TEST_PATH}")

        set_property(TEST ${executable} PROPERTY ENVIRONMENT "PATH=${TEST_PATH}")
    endif()
endfunction()
