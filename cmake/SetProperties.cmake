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

set(DBGA_VERSION "0.0.0.0" CACHE STRING "Version of the DBGA build")

set(REGEX "([0-9]+).([0-9]+).([0-9]+).([0-9]+)")

if(NOT DBGA_VERSION MATCHES "${REGEX}")
    message(SEND_ERROR "Could not match DBGA_VERSION=`${DBGA_VERSION}' with: ${REGEX}")
else()
    set(DBGA_VERSION_MAJOR ${CMAKE_MATCH_1})
    set(DBGA_VERSION_MINOR ${CMAKE_MATCH_2})
    set(DBGA_VERSION_PATCH ${CMAKE_MATCH_3})
    set(DBGA_VERSION_TWEAK ${CMAKE_MATCH_4})
endif()

set(DBGA_NAME "Intel FDK Debug Agent")
set(DBGA_DESCRIPTION "Tool for runtime debugging of CAVS dsp firmware.")
