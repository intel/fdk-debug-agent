/*
 * Copyright (c) 2015, Intel Corporation
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

#include "IfdkObjects/Xml/TypeTraits.hpp"
#include <map>
#include <string>
#include <memory>

namespace debug_agent
{
namespace core
{

/** Main class of instance data model */
class TypeModel
{
public:
    using TypePtr = std::shared_ptr<const ifdk_objects::type::Type>;
    using SystemPtr = std::shared_ptr<const ifdk_objects::type::System>;

    using TypeMap = std::map<std::string, TypePtr>;

    TypeModel(const SystemPtr &systemPtr, const TypeMap &typeMap)
        : mSystemPtr(systemPtr), mTypeMap(typeMap)
    {
    }

    /** @return the system type */
    const SystemPtr getSystem() const { return mSystemPtr; }

    /** @return a type by its name, or nullptr if not found */
    const TypePtr getType(const std::string &typeName) const
    {
        auto it = mTypeMap.find(typeName);
        if (it == mTypeMap.end()) {
            return nullptr;
        }
        return it->second;
    }

    const TypeMap &getTypeMap() const { return mTypeMap; }

private:
    TypeModel(const TypeModel &) = delete;
    TypeModel &operator=(const TypeModel &) = delete;

    SystemPtr mSystemPtr;
    TypeMap mTypeMap;
};
}
}
