# Copyright (c) 2015-2016, Intel Corporation
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
# This is the main cmake file of the DebugAgent project


# cmake configuration file of the "Core" component

# Main executable
set(SRCS
    src/DebugAgent.cpp
    src/Resources.cpp
    src/BaseModelConverter.cpp
    src/InstanceModelConverter.cpp
    src/TypeModelConverter.cpp
    src/DebugResources.cpp
    src/LogServiceParameterApplier.cpp
    src/PerfServiceParameterApplier.cpp
    src/ModuleParameterApplier.cpp
    src/SubsystemParameterApplier.cpp
    src/ProbeServiceParameterApplier.cpp
    src/ProbeEndPointParameterApplier.cpp)

set(INCS
    include/Core/DebugAgent.hpp
    include/Core/Resources.hpp
    include/Core/BaseModelConverter.hpp
    include/Core/InstanceModelConverter.hpp
    include/Core/TypeModelConverter.hpp
    include/Core/InstanceModel.hpp
    include/Core/TypeModel.hpp
    include/Core/DebugResources.hpp
    include/Core/ParameterKind.hpp
    include/Core/ParameterApplier.hpp
    include/Core/ParameterDispatcher.hpp
    include/Core/ServiceParameterApplier.hpp
    include/Core/LogServiceParameterApplier.hpp
    include/Core/PerfServiceParameterApplier.hpp
    include/Core/ModuleParameterApplier.hpp
    include/Core/SubsystemParameterApplier.hpp
    include/Core/ProbeServiceParameterApplier.hpp
    include/Core/EndPointParameterApplier.hpp
    include/Core/ProbeEndPointParameterApplier.hpp

# private headers
    src/FdkToolMockGenerator.hpp
    src/HtmlHelper.hpp
    src/XmlHelper.hpp
)

add_library(Core STATIC ${SRCS} ${INCS})
set_common_settings(Core)

#exporting include directory
target_include_directories(Core PUBLIC "include")

# private headers
target_include_directories(Core PRIVATE "src")

# Binding with Poco
link_poco(Core)

# Needed components
target_link_libraries(Core cAVS Rest Util IfdkObjects ParameterSerializer)
