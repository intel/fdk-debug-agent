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

# cmake configuration file of the "TestCommon" component

# this component intends to share utility classes between unit tests.

# library
set(LIB_SRCS
    src/HttpClientSimulator.cpp)

set(LIB_INCS
    include/TestCommon/HttpClientSimulator.hpp
    include/TestCommon/TestHelpers.hpp
    include/TestCommon/ThreadHelpers.hpp)

add_library(TestCommon STATIC ${LIB_SRCS} ${LIB_INCS})
set_common_settings(TestCommon)

# exporting the include directory
target_include_directories(TestCommon PUBLIC "include")

target_link_libraries(TestCommon Util)

link_poco(TestCommon)

