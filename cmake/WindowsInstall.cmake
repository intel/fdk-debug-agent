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

# Contains windows-specific installation

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
