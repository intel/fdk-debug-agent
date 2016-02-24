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

# debug binary file postfix
set(CMAKE_DEBUG_POSTFIX "_d")

cmake_minimum_required(VERSION 3.2.0)
# 3.3.0 is required to support Visual Studio 2015
if((CMAKE_GENERATOR MATCHES "Visual Studio .*")
   AND (CMAKE_GENERATOR STRGREATER "Visual Studio 14"))
    cmake_minimum_required(VERSION 3.3.0)
endif()

# checking gcc version
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.1")
    message(FATAL_ERROR "Insufficient gcc version")
  endif()
endif()

# Provinding poco library
find_package(Poco REQUIRED COMPONENTS Foundation Net Util Zip)

# Providing parameter framework
find_package(ParameterFramework REQUIRED)

if (NOT ${Poco_FOUND})
    message( FATAL_ERROR "Poco not found : ${Poco_NOT_FOUND_MESSAGE}" )
endif(NOT ${Poco_FOUND})

# Toolchain-specific settings
if (WIN32)
    # warning as error. todo: activate all warnings.
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

    # cmake handles incorrectly static library symbol database, so disabling the involved warning.
    set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099" )

    # windows header define the min/max macros, which conflicts with std::min/max
    # the NOMINMAX allows to not define them
    add_definitions(-DNOMINMAX)

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

# this function creates desktop shortcut and start menu entry
function(create_shortcut FILE_TARGET NAME)
    set_property(INSTALL ${FILE_TARGET} PROPERTY CPACK_DESKTOP_SHORTCUTS ${NAME})
    set_property(INSTALL ${FILE_TARGET} PROPERTY CPACK_START_MENU_SHORTCUTS ${NAME})
endfunction()

# Unfortunately CMAKE_DEBUG_POSTFIX is not used by executable projects, only
# shared library projects. So defining a custom add_executable function
# that appends the postfix.
function (add_dbga_executable projectname)
    add_executable(${projectname} ${ARGN})
    set_target_properties(${projectname} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
endfunction()

# set global DBGA properties (version, name, description...)
include("cmake/SetProperties.cmake")

# We want to *include* this file before adding any test because it calls
# "include(CTest)"
include("ctest/CMakeLists.txt")

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

# Windows specific installation
if (WIN32)
    # installing PFW shared libraries and dependencies
    # note: install(TARGETS... ) cannot be used with imported target
    set (PFW_SHAREDLIBS
        $<TARGET_FILE:ParameterFramework::parameter>
        $<TARGET_FILE:ParameterFramework::remote-processor>
        $<TARGET_FILE:ParameterFramework::libxml2>)
    install(FILES ${PFW_SHAREDLIBS} DESTINATION bin)

    #installing Root PFW configuration files
    install(DIRECTORY resources/parameter-framework DESTINATION .)

    # installing batch files to run the DBGA with specific command-line parameters
    install(FILES
        "resources/scripts/Windows/DebugAgent_d.bat"
        DESTINATION bin CONFIGURATIONS Debug RENAME DebugAgent.bat)
    install(FILES
        "resources/scripts/Windows/DebugAgent.bat"
        DESTINATION bin CONFIGURATIONS Release)
    create_shortcut(bin/DebugAgent.bat "Debug Agent")

    # Creating the directory that will receive xml structure files generated by the fdk tool
    install(DIRECTORY DESTINATION "parameter-framework/cavs")

    # Installing a batch file that opens the cavs xml directory
    install(FILES resources/scripts/Windows/OpenCAVSFolder.bat DESTINATION bin)
    create_shortcut(bin/OpenCAVSFolder.bat "CAVS config")

    # Installing HOWTO file and creating shortcut
    install(FILES "resources/docs/HOWTO.txt" DESTINATION ".")
    create_shortcut(HOWTO.txt "DBGA How to")
endif()

# CPack
add_subdirectory(cpack)


