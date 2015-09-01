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

# cmake configuration file of functional tests

set(TEST_SRCS
    Main.cpp
    CavsTopologySample.cpp)

set(TEST_INCS
    CavsTopologySample.hpp)

# Target specific source and header files
if (WIN32)
    include("CMakeListsWindows.txt")
elseif (UNIX)
    #currently there are no linux specific test
else ()
    error("No cAVS implementation for this platform")
endif ()

add_executable(FunctionalTests ${TEST_SRCS} ${TEST_INCS})
set_common_settings(FunctionalTests)

# include private headers in the same directory as sources
target_include_directories(FunctionalTests PRIVATE ".")

# external libraries
link_catch(FunctionalTests)

# dependent libraries
target_link_libraries(FunctionalTests Core TestCommon cAVSTestFramework)
link_poco(FunctionalTests)

install(TARGETS FunctionalTests RUNTIME DESTINATION tests)
