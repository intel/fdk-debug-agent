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

# cmake configuration file of the "IfdkObjects" component

SET(XML_SRCS
    src/Xml/TypeSerializer.cpp
    src/Xml/TypeDeserializer.cpp
    src/Xml/TypeTraits.cpp)

source_group("Source Files\\Xml" FILES ${XML_SRCS})

SET(TYPE_INCS
    include/IfdkObjects/Type/Type.hpp
    include/IfdkObjects/Type/Component.hpp
    include/IfdkObjects/Type/Subsystem.hpp
    include/IfdkObjects/Type/Ref.hpp
    include/IfdkObjects/Type/TypeRef.hpp
    include/IfdkObjects/Type/ComponentRef.hpp
    include/IfdkObjects/Type/ServiceRef.hpp
    include/IfdkObjects/Type/RefCollection.hpp
    include/IfdkObjects/Type/GenericRefCollection.hpp
    include/IfdkObjects/Type/Categories.hpp
    include/IfdkObjects/Type/Children.hpp
    include/IfdkObjects/Type/TypeRefCollection.hpp
    include/IfdkObjects/Type/ComponentRefCollection.hpp
    include/IfdkObjects/Type/ServiceRefCollection.hpp
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

SET(XML_INCS
    include/IfdkObjects/Xml/Serializer.hpp
    include/IfdkObjects/Xml/Deserializer.hpp
    include/IfdkObjects/Xml/DynamicFactory.hpp
    include/IfdkObjects/Xml/TypeSerializer.hpp
    include/IfdkObjects/Xml/TypeDeserializer.hpp
    include/IfdkObjects/Xml/TypeTraits.hpp)

source_group("Header Files\\Type" FILES ${TYPE_INCS})
source_group("Header Files\\Xml" FILES ${XML_INCS})

set(LIB_SRCS
    ${XML_SRCS})

set(LIB_INCS
    include/IfdkObjects/VisitableVector.hpp
    include/IfdkObjects/VisitablePtrVector.hpp
    ${TYPE_INCS}
    ${XML_INCS})

add_library(IfdkObjects STATIC ${LIB_SRCS} ${LIB_INCS})
set_common_settings(IfdkObjects)

# exporting "include" directory
target_include_directories(IfdkObjects PUBLIC "include")

# external libraries
link_poco(IfdkObjects)
