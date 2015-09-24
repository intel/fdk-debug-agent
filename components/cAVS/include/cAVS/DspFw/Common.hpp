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

#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include <inttypes.h>
#include <iostream>
#include <string>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* All firmware array sizes are stored on 32 bits unsigned integer*/
using ArraySizeType = uint32_t;

/* This compound module id contains a type id and an instance id */
struct CompoundModuleId
{
    uint16_t moduleId;
    uint16_t instanceId;

    bool operator ==(const CompoundModuleId &other) const
    {
        return moduleId == other.moduleId && instanceId == other.instanceId;
    }

    /* Required because this class is used as key of std::map */
    bool operator<(const CompoundModuleId& other) const
    {
        return toInt(*this) < toInt(other);
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        /* In the firmware structure the compound id is store on an uint32_t:
        * - 16 left bits -> module id
        * - 16 right bits-> instance id
        * Therefore the memory mapping is:
        * - first 16 bits: instance id
        * - next 16 bits: module id
        */
        reader.read(instanceId);
        reader.read(moduleId);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(instanceId);
        writer.write(moduleId);
    }

    std::string toString() const
    {
        return "(" + std::to_string(moduleId) + "," + std::to_string(instanceId) + ")";
    }

private:
    static inline uint32_t toInt(const CompoundModuleId &compId)
    {
        return (static_cast<uint32_t>(compId.moduleId) << 16) | compId.instanceId;
    }
};

static std::ostream& operator<< (std::ostream& stream, const CompoundModuleId& id)
{
    stream << id.toString();
    return stream;
}

}
}
}
