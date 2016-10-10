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


# cmake configuration file of the "IfdkObjects" component

SET(XML_SRCS
    src/Xml/TypeSerializer.cpp
    src/Xml/TypeDeserializer.cpp
    src/Xml/TypeTraits.cpp
    src/Xml/InstanceSerializer.cpp
    src/Xml/InstanceDeserializer.cpp
    src/Xml/InstanceTraits.cpp)

source_group("Source Files\\Xml" FILES ${XML_SRCS})

SET(TYPE_INCS
    include/IfdkObjects/Type/Type.hpp
    include/IfdkObjects/Type/Component.hpp
    include/IfdkObjects/Type/Subsystem.hpp
    include/IfdkObjects/Type/System.hpp
    include/IfdkObjects/Type/Service.hpp
    include/IfdkObjects/Type/EndPoint.hpp
    include/IfdkObjects/Type/Ref.hpp
    include/IfdkObjects/Type/TypeRef.hpp
    include/IfdkObjects/Type/ComponentRef.hpp
    include/IfdkObjects/Type/ServiceRef.hpp
    include/IfdkObjects/Type/EndPointRef.hpp
    include/IfdkObjects/Type/SubsystemRef.hpp
    include/IfdkObjects/Type/RefCollection.hpp
    include/IfdkObjects/Type/GenericRefCollection.hpp
    include/IfdkObjects/Type/Categories.hpp
    include/IfdkObjects/Type/Children.hpp
    include/IfdkObjects/Type/TypeRefCollection.hpp
    include/IfdkObjects/Type/SubsystemRefCollection.hpp
    include/IfdkObjects/Type/ComponentRefCollection.hpp
    include/IfdkObjects/Type/ServiceRefCollection.hpp
    include/IfdkObjects/Type/EndPointRefCollection.hpp
    include/IfdkObjects/Type/Characteristics.hpp
    include/IfdkObjects/Type/Characteristic.hpp
    include/IfdkObjects/Type/Description.hpp
    include/IfdkObjects/Type/Connector.hpp
    include/IfdkObjects/Type/Input.hpp
    include/IfdkObjects/Type/Output.hpp
    include/IfdkObjects/Type/Inputs.hpp
    include/IfdkObjects/Type/Outputs.hpp
    include/IfdkObjects/Type/Parameters.hpp
    include/IfdkObjects/Type/ControlParameters.hpp
    include/IfdkObjects/Type/InfoParameters.hpp
    include/IfdkObjects/Type/Visitor.hpp)

SET(INSTANCE_INCS
    include/IfdkObjects/Instance/Instance.hpp
    include/IfdkObjects/Instance/Component.hpp
    include/IfdkObjects/Instance/Subsystem.hpp
    include/IfdkObjects/Instance/System.hpp
    include/IfdkObjects/Instance/Service.hpp
    include/IfdkObjects/Instance/EndPoint.hpp
    include/IfdkObjects/Instance/BaseCollection.hpp
    include/IfdkObjects/Instance/GenericCollection.hpp
    include/IfdkObjects/Instance/InstanceCollection.hpp
    include/IfdkObjects/Instance/ComponentCollection.hpp
    include/IfdkObjects/Instance/SubsystemCollection.hpp
    include/IfdkObjects/Instance/ServiceCollection.hpp
    include/IfdkObjects/Instance/EndPointCollection.hpp
    include/IfdkObjects/Instance/Ref.hpp
    include/IfdkObjects/Instance/InstanceRef.hpp
    include/IfdkObjects/Instance/ComponentRef.hpp
    include/IfdkObjects/Instance/ServiceRef.hpp
    include/IfdkObjects/Instance/EndPointRef.hpp
    include/IfdkObjects/Instance/SubsystemRef.hpp
    include/IfdkObjects/Instance/SystemRef.hpp
    include/IfdkObjects/Instance/RefCollection.hpp
    include/IfdkObjects/Instance/GenericRefCollection.hpp
    include/IfdkObjects/Instance/Children.hpp
    include/IfdkObjects/Instance/InstanceRefCollection.hpp
    include/IfdkObjects/Instance/SubsystemRefCollection.hpp
    include/IfdkObjects/Instance/ComponentRefCollection.hpp
    include/IfdkObjects/Instance/ServiceRefCollection.hpp
    include/IfdkObjects/Instance/EndPointRefCollection.hpp
    include/IfdkObjects/Instance/Connector.hpp
    include/IfdkObjects/Instance/Input.hpp
    include/IfdkObjects/Instance/Output.hpp
    include/IfdkObjects/Instance/Inputs.hpp
    include/IfdkObjects/Instance/Outputs.hpp
    include/IfdkObjects/Instance/Parameters.hpp
    include/IfdkObjects/Instance/ControlParameters.hpp
    include/IfdkObjects/Instance/InfoParameters.hpp
    include/IfdkObjects/Instance/Links.hpp
    include/IfdkObjects/Instance/Link.hpp
    include/IfdkObjects/Instance/From.hpp
    include/IfdkObjects/Instance/To.hpp
    include/IfdkObjects/Instance/Parents.hpp
    include/IfdkObjects/Instance/Visitor.hpp)

SET(XML_INCS
    include/IfdkObjects/Xml/Serializer.hpp
    include/IfdkObjects/Xml/Deserializer.hpp
    include/IfdkObjects/Xml/DynamicFactory.hpp
    include/IfdkObjects/Xml/TypeSerializer.hpp
    include/IfdkObjects/Xml/TypeDeserializer.hpp
    include/IfdkObjects/Xml/TypeTraits.hpp
    include/IfdkObjects/Xml/InstanceSerializer.hpp
    include/IfdkObjects/Xml/InstanceDeserializer.hpp
    include/IfdkObjects/Xml/InstanceTraits.hpp)

source_group("Header Files\\Type" FILES ${TYPE_INCS})
source_group("Header Files\\Instance" FILES ${INSTANCE_INCS})
source_group("Header Files\\Xml" FILES ${XML_INCS})

set(LIB_SRCS
    ${XML_SRCS})

set(LIB_INCS
    include/IfdkObjects/VisitableVector.hpp
    include/IfdkObjects/VisitablePtrVector.hpp
    ${TYPE_INCS}
    ${INSTANCE_INCS}
    ${XML_INCS})

add_library(IfdkObjects STATIC ${LIB_SRCS} ${LIB_INCS})
set_common_settings(IfdkObjects)

# exporting "include" directory
target_include_directories(IfdkObjects PUBLIC "include")

# external libraries
link_poco(IfdkObjects)

# other dependencies
target_link_libraries(IfdkObjects Util)

# adding test
add_subdirectory("test")

