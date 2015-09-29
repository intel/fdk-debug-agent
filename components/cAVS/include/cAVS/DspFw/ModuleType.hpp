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

#include "cAVS/DspFw/Common.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

union SegmentFlags
{
    uint32_t    ul;
    struct
    {
        uint32_t    contents : 1;
        uint32_t    alloc : 1;
        uint32_t    load : 1;
        uint32_t    readonly : 1;
        uint32_t    code : 1;
        uint32_t    data : 1;
        uint32_t    _rsvd0 : 2;

        uint32_t    type : 4;
        uint32_t    _rsvd1 : 4;

        uint32_t    length : 16; // segment length in pages
    } r;

    bool operator == (const SegmentFlags &other) const
    {
        return ul == other.ul;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(ul);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(ul);
    }
};
static_assert(sizeof(SegmentFlags) == 4, "Wrong SegmentFlags size");

struct SegmentDesc
{
    SegmentFlags    flags;
    uint32_t        v_base_addr;
    uint32_t        file_offset;

    bool operator == (const SegmentDesc &other) const
    {
        return flags == other.flags &&
            v_base_addr == other.v_base_addr &&
            file_offset == other.file_offset;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        flags.fromStream(reader);
        reader.read(v_base_addr);
        reader.read(file_offset);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        flags.toStream(writer);
        writer.write(v_base_addr);
        writer.write(file_offset);
    }
};
static_assert(sizeof(SegmentDesc) == 12, "Wrong SegmentDesc size");

union ModuleType
{
    uint32_t    ul;
    struct
    {
        uint32_t     load_type : 4; // MT_BUILTIN, MT_LOADABLE
        uint32_t     auto_start : 1; // 0 - manually created, 1 - single instance created by
                                     // Module Manager
        uint32_t     domain_ll : 1; // support LL domain
        uint32_t     domain_dp : 1; // support DP domain
        uint32_t     _rsvd : 25;
    } r;

    bool operator == (const ModuleType &other) const
    {
        return ul == other.ul;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(ul);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(ul);
    }
};
static_assert(sizeof(ModuleType) == 4, "Wrong ModuleType size");

struct ModuleEntry
{
private:
    template <typename T>
    static bool isArrayEqual(const T* a1, const T* a2, std::size_t size)
    {
        for (std::size_t i = 0; i < size; i++) {
            if (!(a1[i] == a2[i])) {
                return false;
            }
        }
        return true;
    }

public:
    static const std::size_t SEGMENT_COUNT = 3;
    static const std::size_t MAX_MODULE_NAME_LEN = 8;
    static const std::size_t DEFAULT_HASH_SHA256_LEN = 32;
    static const std::size_t UUID_LEN = 4;

    uint32_t    struct_id;
    uint8_t     name[MAX_MODULE_NAME_LEN];
    uint32_t    uuid[UUID_LEN];
    ModuleType  type;                           // ModuleType
    uint8_t     hash[DEFAULT_HASH_SHA256_LEN];
    uint32_t    entry_point;
    uint16_t    cfg_offset;
    uint16_t    cfg_count;
    uint32_t    affinity_mask;              // bit-mask of cores allowed to exec module
    uint16_t    instance_max_count;         // max number of instances
    uint16_t    instance_stack_size;        // size of stack that instance requires for its task
                                            // (DP) [bytes]
    SegmentDesc segments[SEGMENT_COUNT];

    bool operator == (const ModuleEntry &other) const
    {
        return struct_id == other.struct_id &&
            isArrayEqual(name, other.name, MAX_MODULE_NAME_LEN) &&
            isArrayEqual(uuid, other.uuid, UUID_LEN) &&
            type == other.type &&
            isArrayEqual(hash, other.hash, DEFAULT_HASH_SHA256_LEN) &&
            entry_point == other.entry_point &&
            cfg_offset == other.cfg_offset &&
            cfg_count == other.cfg_count &&
            affinity_mask == other.affinity_mask &&
            instance_max_count == other.instance_max_count &&
            instance_stack_size == other.instance_stack_size &&
            isArrayEqual(segments, other.segments, SEGMENT_COUNT);
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(struct_id);
        reader.readArray(name, MAX_MODULE_NAME_LEN);
        reader.readArray(uuid, UUID_LEN);
        type.fromStream(reader);
        reader.readArray(hash, DEFAULT_HASH_SHA256_LEN);
        reader.read(entry_point);
        reader.read(cfg_offset);
        reader.read(cfg_count);
        reader.read(affinity_mask);
        reader.read(instance_max_count);
        reader.read(instance_stack_size);

        for (std::size_t i = 0; i < SEGMENT_COUNT; i++) {
            segments[i].fromStream(reader);
        }
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(struct_id);
        writer.writeArray(name, MAX_MODULE_NAME_LEN);
        writer.writeArray(uuid, UUID_LEN);
        type.toStream(writer);
        writer.writeArray(hash, DEFAULT_HASH_SHA256_LEN);
        writer.write(entry_point);
        writer.write(cfg_offset);
        writer.write(cfg_count);
        writer.write(affinity_mask);
        writer.write(instance_max_count);
        writer.write(instance_stack_size);

        for (std::size_t i = 0; i < SEGMENT_COUNT; i++) {
            segments[i].toStream(writer);
        }
    }
};
static_assert(sizeof(ModuleEntry) == 116, "Wrong ModuleEntry size");

struct ModulesInfo
{
    std::vector<ModuleEntry> module_info;

    static std::size_t getAllocationSize(std::size_t count) {
        return sizeof(ArraySizeType)+count * sizeof(ModuleEntry);
    }

    bool operator == (const ModulesInfo &other)
    {
        return module_info == other.module_info ;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVectorAndRecurse<ArraySizeType>(module_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVectorAndRecurse<ArraySizeType>(module_info);
    }
};

}
}
}
