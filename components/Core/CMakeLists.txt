################################################################################
#                              INTEL CONFIDENTIAL
#   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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

# cmake configuration file of the "Core" component

# Main executable
set(SRCS
    src/DebugAgent.cpp
    src/Resources.cpp
    src/BaseModelConverter.cpp
    src/InstanceModelConverter.cpp
    src/TypeModelConverter.cpp
    src/ModuleResources.cpp
    src/DebugResources.cpp
    src/LogServiceParameterApplier.cpp)

set(INCS
    include/Core/DebugAgent.hpp
    include/Core/Resources.hpp
    include/Core/BaseModelConverter.hpp
    include/Core/InstanceModelConverter.hpp
    include/Core/TypeModelConverter.hpp
    include/Core/InstanceModel.hpp
    include/Core/TypeModel.hpp
    include/Core/ModuleResources.hpp
    include/Core/DebugResources.hpp
    include/Core/ParameterKind.hpp
    include/Core/ParameterApplier.hpp
    include/Core/ParameterDispatcher.hpp
    include/Core/LogServiceParameterApplier.hpp

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
