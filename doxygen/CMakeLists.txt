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

if(DOXYGEN)

    find_package(Doxygen REQUIRED)

    if(NOT DOXYGEN_DOT_FOUND)
        message(SEND_ERROR "The 'dot' utility was  not found")
    endif()

    set(DOXYGEN_SEARCHENGINE YES)
    set(DOT_IMAGE_FORMAT svg)
    if(WIN32)
        # On windows, generate a CHM file
        set(DOXYGEN_GENERATE_HTMLHELP YES)
        # The html search engin must be disabled when generating a CHM file
        set(DOXYGEN_SEARCHENGINE NO)
        set(DOXYGEN_HHC_LOCATION "C:/Program Files (x86)/HTML Help Workshop/hhc.exe")
        # There seems to be an issue with plantuml-generated SVG diagrams on the CHM viewer. Switch
        # to PNG instead.
        set(DOT_IMAGE_FORMAT png)
    endif()

    # Configures path information and some variables defined by `find_package(Doxygen)`
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
       ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
       @ONLY)
    add_custom_target(doxygen
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM)
endif()
