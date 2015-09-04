/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
*   The source code contained  or  described herein and all documents related to
*   the source code ("Material") are owned by Intel Corporation or its suppliers
*   or licensors.  Title to the  Material remains with  Intel Corporation or its
*   suppliers and licensors. The Material contains trade secrets and proprietary
*   and  confidential  information of  Intel or its suppliers and licensors. The
*   Material  is  protected  by  worldwide  copyright  and trade secret laws and
*   treaty  provisions. No part of the Material may be used, copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way without Intel's prior express written permission.
*   No license  under any  patent, copyright, trade secret or other intellectual
*   property right is granted to or conferred upon you by disclosure or delivery
*   of the Materials,  either expressly, by implication, inducement, estoppel or
*   otherwise.  Any  license  under  such  intellectual property  rights must be
*   express and approved by Intel in writing.
*
********************************************************************************
*/

#pragma once

#include "Core/BaseModelConverter.hpp"
#include "Core/TypeModel.hpp"
#include "cAVS/System.hpp"

namespace debug_agent
{
namespace core
{

/** This class converts cAVS data model to generic type data model */
class TypeModelConverter final : public BaseModelConverter
{
public:
    TypeModelConverter(cavs::System &system) : BaseModelConverter(system) {}

    std::shared_ptr<TypeModel> createModel();

private:
    std::shared_ptr<ifdk_objects::type::System> createSystem();
    std::shared_ptr<ifdk_objects::type::Type> createSubsystem();
    std::shared_ptr<ifdk_objects::type::Type> createPipe();
    std::shared_ptr<ifdk_objects::type::Type> createTask();
    std::shared_ptr<ifdk_objects::type::Type> createCore();
    std::shared_ptr<ifdk_objects::type::Type> createGateway(const std::string &name);
    std::shared_ptr<ifdk_objects::type::Type> createModule(uint32_t id);

    /** Get system characteristics */
    void getSystemCharacteristics(ifdk_objects::type::Characteristics &characteristics);

    /** Add one type owned by a subsystem in the supplied map. The key of the map is :
     *  <subsystem name>.<type name>
     */
    static void addSubsystemSubType(TypeModel::TypeMap &map,
        std::shared_ptr<ifdk_objects::type::Type> type);
};

}
}


