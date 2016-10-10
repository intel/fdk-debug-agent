/*
 * Copyright (c) 2015-2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
    std::shared_ptr<ifdk_objects::type::Type> createModule(uint16_t id);

    /** Get system characteristics */
    void getSystemCharacteristics(ifdk_objects::type::Characteristics &characteristics);

    /** Add one type owned by a subsystem in the supplied map. The key of the map is :
     *  <subsystem name>.<type name>
     */
    static void addSubsystemSubType(TypeModel::TypeMap &map,
                                    std::shared_ptr<ifdk_objects::type::Type> type);

    /* Common method to create service type */
    static std::shared_ptr<ifdk_objects::type::Type> createService(
        const std::string &serviceTypeName, std::size_t endPointCount);

    /* Common method to create endpoint type */
    static std::shared_ptr<ifdk_objects::type::Type> createEndPoint(
        const std::string &serviceTypeName, ifdk_objects::type::EndPoint::Direction direction);

    /* Add all types required by a service */
    static void addSubsystemServiceTypes(TypeModel::TypeMap &map,
                                         const std::string &serviceTypeName,
                                         ifdk_objects::type::EndPoint::Direction direction,
                                         std::size_t endPointCount);
};
}
}
