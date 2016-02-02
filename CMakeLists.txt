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

# 3.3.0 is required to support Visual Studio 2015
cmake_minimum_required(VERSION 3.3.0)

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
    set_property(TARGET ${projectname} APPEND PROPERTY CXX_EXTENSIONS OFF)
    set_property(TARGET ${projectname} APPEND PROPERTY CXX_STANDARD_REQUIRED ON)
endfunction()

# We want to *include* this file before adding any test because it calls
# "include(CTest)"
include("ctest/CMakeLists.txt")

# Adding imported external libraries
add_subdirectory("external/cmake")

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

# will contains parameter-framework targets
set(PFW_TARGETS CACHE INTERNAL "PFW_TARGETS")

# this function adds a parameter-framework target if it is not imported
function (add_pfw_target name)
    get_target_property(isImported ${name} IMPORTED)
    if (NOT ${isImported})
        set(PFW_TARGETS ${PFW_TARGETS} ${name} CACHE INTERNAL "PFW_TARGETS")
    endif()
endfunction()

# hooking add_library function to register parameter-framework libraries
function (add_library name)
  _add_library (${name} ${ARGN})
  add_pfw_target(${name})
endfunction ()

# hooking add_executable function to register parameter-framework executables
function (add_executable name)
  _add_executable (${name} ${ARGN})
  add_pfw_target(${name})
endfunction ()

# including pfw projects
add_subdirectory("external/parameter-framework/project")

# creating a custom target "all_pfw" that depends of all pfw targets
add_custom_target(pfw_all)
add_dependencies(pfw_all ${PFW_TARGETS})

# Restore the previous value of BUILD_TESTING and CMAKE_CXX_FLAGS since we
# overrode them for the Parameter Framework but don't want to mess up our own
# cache.
set(BUILD_TESTING "${OLD_BUILD_TESTING}" CACHE BOOL "" FORCE)
set(CMAKE_CXX_FLAGS "${OLD_CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)

# installing libxml2 shared library into bin/ directories
install(FILES $<TARGET_FILE:libxml2::deployment> DESTINATION bin)


