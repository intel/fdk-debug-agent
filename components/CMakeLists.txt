################################################################################
#                              INTEL CONFIDENTIAL
#   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

if (UNIX)
    # unused-parameter errors in Release mode stem from parameters being only
    # used in assert() calls, which are disabled in that mode.
    set_directory_properties(PROPERTIES
        COMPILE_OPTIONS "-Wall;$<$<CONFIG:Release>:-Wno-unused-parameter>;-Wextra")
endif()

# Adding components in the right order (less dependent first)
foreach(dir Util IfdkObjects TestCommon Tlv Rest System cAVS ParameterSerializer Core Main)
    add_subdirectory("${dir}")
endforeach()

if (NOT ${EXCLUDE_TEST_IN_ALL_TARGET})
    add_subdirectory("FunctionalTests")
endif (NOT ${EXCLUDE_TEST_IN_ALL_TARGET})

# Windows only components
if (WIN32)
    add_subdirectory("LogGrabber")
endif()

