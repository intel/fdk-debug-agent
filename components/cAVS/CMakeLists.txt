################################################################################
#                              INTEL CONFIDENTIAL
#   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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

# option to use Alsa-lib based Control device
SET_PROPERTY(CACHE MIXER_CONTROL_LIB_TO_USE PROPERTY STRINGS Tinyalsa Alsa-lib Stub STRING
    "library selected to set/get control mixer on soundcard")

# Common source files
set(LIB_SRCS
    src/ModuleHandler.cpp
    src/LogStreamer.cpp
    src/System.cpp
    src/Topology.cpp
    src/ProbeService.cpp)

# Common include files
set(LIB_INCS
    include/cAVS/LogBlock.hpp
    include/cAVS/Logger.hpp
    include/cAVS/LogStreamer.hpp
    include/cAVS/Driver.hpp
    include/cAVS/DriverFactory.hpp
    include/cAVS/SystemDriverFactory.hpp
    include/cAVS/System.hpp
    include/cAVS/ModuleHandler.hpp
    include/cAVS/Topology.hpp
    include/cAVS/Prober.hpp
    include/cAVS/ProbeService.hpp)

# firwmare include files
set(FW_INCS
    include/cAVS/DspFw/AudioFormat.hpp
    include/cAVS/DspFw/ConfigTypes.hpp
    include/cAVS/DspFw/FwConfig.hpp
    include/cAVS/DspFw/HwConfig.hpp
    include/cAVS/DspFw/ModuleInstance.hpp
    include/cAVS/DspFw/Pipeline.hpp
    include/cAVS/DspFw/Common.hpp
    include/cAVS/DspFw/Gateway.hpp
    include/cAVS/DspFw/Infrastructure.hpp
    include/cAVS/DspFw/ModuleType.hpp
    include/cAVS/DspFw/Scheduler.hpp
    include/cAVS/DspFw/ExternalFirmwareHeaders.hpp
    include/cAVS/DspFw/Probe.hpp)

source_group("Header Files\\DspFw" FILES ${FW_INCS})
set(LIB_INCS ${LIB_INCS} ${FW_INCS})

set(LIBRAIRIES)

# Target specific source and header files
if (WIN32)
    # Windows IOCTL SST Driver
    include("CMakeListsWindows.txt")

    # including also Linux sources, but excluding them from build
    include("CMakeListsLinux.txt")
    set_source_files_properties(${LINUX_LIB_SRCS} PROPERTIES HEADER_FILE_ONLY true)

    # adding also os-specific cmake files to the visual studio project
    set(LIB_SRCS ${LIB_SRCS} CMakeListsWindows.txt CMakeListsLinux.txt)
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

find_path(CAVS_FW_HEADERS_INCLUDE_DIRECTORIES FW/src/common/include/stream_type.h)

# including and exporting cAVS fw headers
target_include_directories(cAVS PUBLIC
    "${CAVS_FW_HEADERS_INCLUDE_DIRECTORIES}/FW/src/common/include"
    "${CAVS_FW_HEADERS_INCLUDE_DIRECTORIES}/FW/src/intel_adsp/include"
)

if (WIN32)
    # including and exporting OED driver headers
    target_include_directories(cAVS PUBLIC
        "${PROJECT_SOURCE_DIR}/external/cAVS/cavs_sw/SW/OEPrivIface")
endif()

link_poco(cAVS)

#Adding test framework library, currently windows only
add_subdirectory("TestFramework")

# Adding test
add_subdirectory("test")

# Needed components
target_link_libraries(cAVS System Util Tlv)

if (CMAKE_SYSTEM_NAME MATCHES Linux)
    find_path(TINYCOMPRESS_INCLUDE_DIR NAMES tinycompress/tinycompress.h REQUIRED)
    target_include_directories(cAVS PUBLIC "${TINYCOMPRESS_INCLUDE_DIR}")

    find_library(TINYCOMPRESS_LIBRARY NAMES libtinycompress.so REQUIRED)
    target_link_libraries(cAVS ${TINYCOMPRESS_LIBRARY})
endif ()
