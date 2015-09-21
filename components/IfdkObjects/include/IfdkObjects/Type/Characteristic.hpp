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

#include "IfdkObjects/Type/Visitor.hpp"
#include <string>
#include <cassert>

namespace debug_agent
{
namespace ifdk_objects
{
namespace type
{

/** This class is a simple key/value pair */
class Characteristic
{
public:
    Characteristic() = default;
    explicit Characteristic(const std::string& name) : mName(name) {}
    explicit Characteristic(const std::string& name, const std::string& value) :
        mName(name), mValue(value) {}
    explicit Characteristic(const Characteristic& other) = default;
    virtual ~Characteristic() = default;
    Characteristic &operator=(const Characteristic &other) = default;

    bool operator == (const Characteristic &other) const NOEXCEPT
    {
        return mName == other.mName && mValue == other.mValue;
    }

    bool operator != (const Characteristic &other) const NOEXCEPT
    {
        return !(*this == other);
    }

    void accept(Visitor &visitor)
    {
        acceptCommon(*this, visitor);
    }

    void accept(ConstVisitor &visitor) const
    {
        acceptCommon(*this, visitor);
    }

    std::string getName() const
    {
        return mName;
    }

    void setName(const std::string &name)
    {
        mName = name;
    }

    std::string getValue() const
    {
        return mValue;
    }

    void setValue(const std::string &value)
    {
        mValue = value;
    }

private:
    template <typename T, typename Visitor>
    static void acceptCommon(T &me, Visitor &visitor)
    {
        visitor.enter(me);
        visitor.leave();
    }

    std::string mName;
    std::string mValue;
};

}
}
}


