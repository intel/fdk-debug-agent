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

# cmake configuration file of the "cAVS" component


# Common source files
set(LIB_SRCS
    src/Logger.cpp
    src/LogStreamer.cpp
    src/System.cpp)

# Common include files
set(LIB_INCS
    include/cAVS/LogBlock.hpp
    include/cAVS/Logger.hpp
    include/cAVS/LogStreamer.hpp
    include/cAVS/Driver.hpp
    include/cAVS/DriverFactory.hpp
    include/cAVS/SystemDriverFactory.hpp
    include/cAVS/System.hpp
    include/cAVS/FirmwareTypes.hpp
    include/cAVS/ModuleHandler.hpp
    include/cAVS/FwConfig.hpp
    include/cAVS/HwConfig.hpp)

set(LIBRAIRIES)
SET(INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/external/cAVS/firmware/include")

# Target specific source and header files
if (WIN32)
    # Windows IOCTL SST Driver
    include("CMakeListsWindows.txt")
elseif (UNIX)
    # Linux Debug FS SST Driver
    include("CMakeListsLinux.txt")
else ()
    error("No cAVS implementation for this platform")
endif (WIN32)

add_library(cAVS STATIC ${LIB_SRCS} ${LIB_INCS})
set_common_settings(cAVS)

target_link_libraries(cAVS ${LIBRAIRIES})

# exporting "include" directory
target_include_directories(cAVS PUBLIC "include")

# Adding include directories
target_include_directories(cAVS PUBLIC ${INCLUDE_DIRS})

link_poco(cAVS)

#Adding test framework library, currently windows only
add_subdirectory("TestFramework")

# Adding test
add_subdirectory("test")

# Needed components
target_link_libraries(cAVS System Util Tlv)
