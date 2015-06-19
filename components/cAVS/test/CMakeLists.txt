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

# cmake configuration file of the "cAVS" component unit test

# test
set(TEST_SRCS
    LoggerUnitTest.cpp)

set(TEST_INCS)

# Target specific source and header files
if (WIN32)
    include("CMakeListsWindows.txt")
elseif (UNIX)
    #currently there are no linux specific test
else ()
    error("No cAVS implementation for this platform")
endif ()

add_executable(CavsUnitTest ${TEST_SRCS} ${TEST_INCS})
set_common_settings(CavsUnitTest)

# external libraries
link_catch(CavsUnitTest)

# Binding with cAVS library
target_link_libraries(CavsUnitTest cAVS)

# Binding with test common in order to use the HttpClientSimulator
target_link_libraries(CavsUnitTest TestCommon)

install(TARGETS CavsUnitTest RUNTIME DESTINATION tests)
