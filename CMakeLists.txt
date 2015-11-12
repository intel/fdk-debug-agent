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

# This is the main cmake file of the DebugAgent project

project(DebugAgent)

cmake_minimum_required(VERSION 3.0.0)

# checking gcc version
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.1")
    message(FATAL_ERROR "Insufficient gcc version")
  endif()
endif()

# Provinding poco library
find_package(Poco REQUIRED COMPONENTS Foundation Net Util Zip)

if (NOT ${Poco_FOUND})
    message( FATAL_ERROR "Poco not found : ${Poco_NOT_FOUND_MESSAGE}" )
endif(NOT ${Poco_FOUND})

# Toolchain-specific settings
if (WIN32)
    # warning as error. todo: activate all warnings.
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

    # cmake handles incorrectly static library symbol database, so disabling the involved warning.
    set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099" )
elseif (CMAKE_COMPILER_IS_GNUCXX)
    # warning as error. todo: activate all warnings.
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
else ()
    message( FATAL_ERROR "Unsupported toolchain." )
endif()

# usage: link_poco(<target_name>) where target_name is the target that links with poco.
function(link_poco target)
    target_link_libraries(${target} ${Poco_LIBRARIES})

    # POCO_STATIC -> using poco as static libraries
    # POCO_NO_AUTOMATIC_LIBS -> disable an ugly feature of visual studio c++ : the ability to
    #                           link with static libraries through the code using #pragma.
    #                           In this way the behavior is the same between Linux and Windows
    set_property( TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS
        POCO_STATIC=1 POCO_NO_AUTOMATIC_LIBS=1)
endfunction()

# providing "catch" test framework library
# usage: link_catch(<target_name>) where target_name is the target that links with catch.
function(link_catch target)
    target_include_directories(${target} SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/external/catch/include")
endfunction()

# this function applies common settings to all debug agent projects
function (set_common_settings projectname)
    #enabling c++14
    set_property(TARGET ${projectname} APPEND PROPERTY CXX_STANDARD 14)
    set_property(TARGET ${projectname} APPEND PROPERTY CXX_STANDARD_REQUIRED ON)
if (WIN32)
    # Windows does not support noexcept: fall back to throw()
    set_property(TARGET ${projectname} APPEND PROPERTY COMPILE_DEFINITIONS "NOEXCEPT=throw()")
else ()
    set_property(TARGET ${projectname} APPEND PROPERTY COMPILE_DEFINITIONS NOEXCEPT=noexcept)
endif ()
endfunction()

# Adding components in the right order (less dependent first)
add_subdirectory("components/Util")
add_subdirectory("components/IfdkObjects")
add_subdirectory("components/TestCommon")
add_subdirectory("components/Tlv")
add_subdirectory("components/Rest")
add_subdirectory("components/System")
add_subdirectory("components/cAVS")
add_subdirectory("components/ParameterSerializer")
add_subdirectory("components/Core")
add_subdirectory("components/Main")
add_subdirectory("components/FunctionalTests")

# Windows only components
if (WIN32)
    add_subdirectory("components/LogGrabber")
endif()

#
# Add the Parameter Framework project
#

# Don't test the Parameter Framework in the context of the Debug Agent
set(OLD_BUILD_TESTING ${BUILD_TESTING})
set(BUILD_TESTING off CACHE BOOL "" FORCE)

# No need for the bindings
set(PYTHON_BINDINGS off CACHE BOOL "" FORCE)
set(C_BINDINGS off CACHE BOOL "" FORCE)

# Backup and reset cmake flags
set(OLD_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "")

add_subdirectory("external/parameter-framework")

# Restore the previous value of BUILD_TESTING and CMAKE_CXX_FLAGS since we
# overrode them for the Parameter Framework but don't want to mess up our own
# cache.
set(BUILD_TESTING "${OLD_BUILD_TESTING}" CACHE BOOL "" FORCE)
set(CMAKE_CXX_FLAGS "${OLD_CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)

#
# Installation of shared libraries
#

if(WIN32)
    set(LIBXML2_SHARED_LIB ${PROJECT_SOURCE_DIR}/external/libxml2/Win64/bin/libxml2.dll)
else()
    set(LIBXML2_SHARED_LIB ${PROJECT_SOURCE_DIR}/external/libxml2/Linux64/lib/libxml2.so)
endif()

# installing libxml2 shared library into bin/ and test/ directories
install(FILES ${LIBXML2_SHARED_LIB} DESTINATION bin)
install(FILES ${LIBXML2_SHARED_LIB} DESTINATION tests)

# installing parameter-framework and remote-processor shared libraries in test/ directory
# note: these shared libraries are already installed into the bin/ directory.
install(FILES $<TARGET_FILE:parameter> $<TARGET_FILE:remote-processor> DESTINATION tests)


