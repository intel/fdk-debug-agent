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

#include "IfdkObjects/Instance/Children.hpp"
#include "IfdkObjects/Instance/InfoParameters.hpp"
#include "IfdkObjects/Instance/ControlParameters.hpp"
#include "IfdkObjects/Instance/Parents.hpp"
#include <string>

namespace debug_agent
{
namespace ifdk_objects
{
namespace instance
{

/** Base instance class
 *
 * This class introduces:
 * - type name
 * - instance id
 * - info parameters
 * - control parameters
 * - parents
 * - children
 */
class Instance
{
public:
    Instance() = default;
    explicit Instance(const std::string& typeName, const std::string &instanceId) :
        mTypeName(typeName), mInstanceId(instanceId) {}
    explicit Instance(const Instance& other) = default;
    virtual ~Instance() = default;
    Instance &operator=(const Instance &other) = default;

    bool operator == (const Instance &other) const NOEXCEPT
    {
        if (typeid(*this) != typeid(other)) {
            return false;
        }

        return equalsTo(other);
    }

    bool operator != (const Instance &other) const NOEXCEPT
    {
        return !(*this == other);
    }

    virtual void accept(Visitor &visitor, bool isConcrete = true)
    {
        acceptCommon(*this, visitor, isConcrete);
    }

    virtual void accept(ConstVisitor &visitor, bool isConcrete = true) const
    {
        acceptCommon(*this, visitor, isConcrete);
    }

    std::string getTypeName() const
    {
        return mTypeName;
    }

    void setTypeName(const std::string &typeName)
    {
        mTypeName = typeName;
    }

    std::string getInstanceId() const
    {
        return mInstanceId;
    }

    void setInstanceId(const std::string &instanceId)
    {
        mInstanceId = instanceId;
    }

    InfoParameters &getInfoParameters()
    {
        return mInfoParameters;
    }

    ControlParameters &getControlParameters()
    {
        return mControlParameters;
    }

    Parents &getParents()
    {
        return mParents;
    }

    Children &getChildren()
    {
        return mChildren;
    }

protected:
    virtual bool equalsTo(const Instance &other) const NOEXCEPT
    {
        return
            mTypeName == other.mTypeName &&
            mInstanceId == other.mInstanceId &&
            mInfoParameters == other.mInfoParameters &&
            mControlParameters == other.mControlParameters &&
            mParents == other.mParents &&
            mChildren == other.mChildren;
    }

private:
    template <typename T, typename Visitor>
    static void acceptCommon(T &me, Visitor &visitor, bool isConcrete)
    {
        visitor.enter(me, isConcrete);

        me.mInfoParameters.accept(visitor);
        me.mControlParameters.accept(visitor);
        me.mParents.accept(visitor);
        me.mChildren.accept(visitor);

        visitor.leave(isConcrete);
    }

    std::string mTypeName;
    std::string mInstanceId;
    InfoParameters mInfoParameters;
    ControlParameters mControlParameters;
    Parents mParents;
    Children mChildren;
};

}
}
}


