# Copyright (c) 2016, Intel Corporation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

set(CPACK_PACKAGE_VERSION
    "${DBGA_VERSION_MAJOR}.${DBGA_VERSION_MINOR}.${DBGA_VERSION_PATCH}.${DBGA_VERSION_TWEAK}")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${DBGA_DESCRIPTION})
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/resources/docs/DebugAgentLicence.rtf")
set(CPACK_PACKAGE_VENDOR "Intel")

# Arbitrary GUID needed by the installer on windows to abort
# if it detects a pre-existing installation that uses the same GUID.
set(CPACK_WIX_PRODUCT_GUID "6E4C7E7F-23FC-4CA8-AC62-E149D5710269")

# Arbitrary GUID needed by the installer on windows to replace
# existing installations that use the same GUID.
set(CPACK_WIX_UPGRADE_GUID "49A98B9B-BB0B-44BE-8ECE-3584E9EE83D7")

# Use dpkg-shlibdeps (debian generator only) to generate better package dependency list.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

if (NOT ENABLE_ADMIN_STEPS)
    # Wix uses Microsoft ICE validation to validate installer files
    # see https://msdn.microsoft.com/en-us/library/windows/desktop/aa369206(v=vs.85).aspx
    # But the ICE validation requires admin rights. So disabling it by setting the "-sval"
    # parameter of the Wix "light" executable.
    #
    # More info about the issue:
    # http://stackoverflow.com/questions/1064580/wix-3-0-throws-error-217-while-being-executed-by-continuous-integration
    set(CPACK_WIX_LIGHT_EXTRA_FLAGS "-sval")
endif()

include(CPack)
