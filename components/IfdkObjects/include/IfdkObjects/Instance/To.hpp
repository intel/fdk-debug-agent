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

#include "IfdkObjects/Instance/Visitor.hpp"
#include <string>

namespace debug_agent
{
namespace ifdk_objects
{
namespace instance
{

/* Link destination */
class To
{
public:
    To() = default;
    explicit To(const std::string &typeName, const std::string &instanceId,
                const std::string &inputId)
        : mTypeName(typeName), mInstanceId(instanceId), mInputId(inputId)
    {
    }
    explicit To(const To &other) = default;
    To &operator=(const To &other) = default;

    bool operator==(const To &other) const NOEXCEPT
    {
        return mTypeName == other.mTypeName && mInstanceId == other.mInstanceId &&
               mInputId == other.mInputId;
    }

    bool operator!=(const To &other) const NOEXCEPT { return !(*this == other); }

    virtual void accept(Visitor &visitor) { acceptCommon(*this, visitor); }

    virtual void accept(ConstVisitor &visitor) const { acceptCommon(*this, visitor); }

    std::string getTypeName() const { return mTypeName; }

    void setTypeName(const std::string &typeName) { mTypeName = typeName; }

    std::string getInstanceId() const { return mInstanceId; }

    void setInstanceId(const std::string &instanceId) { mInstanceId = instanceId; }

    std::string getInputId() const { return mInputId; }

    void setInputId(const std::string &inputId) { mInputId = inputId; }

private:
    template <typename T, typename Visitor>
    static void acceptCommon(T &me, Visitor &visitor)
    {
        visitor.enter(me);
        visitor.leave(true);
    }

    std::string mTypeName;
    std::string mInstanceId;
    std::string mInputId;
};
}
}
}
