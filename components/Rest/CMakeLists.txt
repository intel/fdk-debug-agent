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

# cmake configuration file of the "Rest" component

# library
set(LIB_SRCS
    src/Server.cpp
    src/Dispatcher.cpp
    src/ServerRequestHandling.cpp
    src/Resource.cpp)

set(LIB_INCS
    include/Rest/Server.hpp
    include/Rest/Request.hpp
    include/Rest/Response.hpp
    include/Rest/Dispatcher.hpp
    include/Rest/Resource.hpp
    include/Rest/HttpMessageProperties.hpp
    include/Rest/ErrorHandler.hpp
    src/ServerRequestHandling.hpp) # private header

add_library(Rest STATIC ${LIB_SRCS} ${LIB_INCS})
set_common_settings(Rest)

# including private headers in src directory
target_include_directories(Rest PRIVATE "src")

# exporting "include" directory
target_include_directories(Rest PUBLIC "include")

# external libraries
link_poco(Rest)

# adding test
add_subdirectory("test")

# Needed components
target_link_libraries(Rest Util)

