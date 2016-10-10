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

#include "cAVS/DspFw/Common.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/StructureChangeTracking.hpp"
#include "Util/StringHelper.hpp"

#include <array>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* SegmentFlags */

CHECK_SIZE(private_fw::SegmentFlags, 4);
CHECK_MEMBER(private_fw::SegmentFlags, ul, 0, uint32_t);

union SegmentFlags
{
    uint32_t ul;
    struct
    {
        uint32_t contents : 1;
        uint32_t alloc : 1;
        uint32_t load : 1;
        uint32_t readonly : 1;
        uint32_t code : 1;
        uint32_t data : 1;
        uint32_t _rsvd0 : 2;

        uint32_t type : 4;
        uint32_t _rsvd1 : 4;

        uint32_t length : 16; // segment length in pages
    } r;

    bool operator==(const SegmentFlags &other) const { return ul == other.ul; }

    void fromStream(util::ByteStreamReader &reader) { reader.read(ul); }

    void toStream(util::ByteStreamWriter &writer) const { writer.write(ul); }
};

/* SegmentDesc */

CHECK_SIZE(private_fw::SegmentDesc, 12);
CHECK_MEMBER(private_fw::SegmentDesc, flags, 0, private_fw::SegmentFlags);
CHECK_MEMBER(private_fw::SegmentDesc, v_base_addr, 4, uint32_t);
CHECK_MEMBER(private_fw::SegmentDesc, file_offset, 8, uint32_t);

struct SegmentDesc
{
    SegmentFlags flags;
    uint32_t v_base_addr;
    uint32_t file_offset;

    bool operator==(const SegmentDesc &other) const
    {
        return flags == other.flags && v_base_addr == other.v_base_addr &&
               file_offset == other.file_offset;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(flags);
        reader.read(v_base_addr);
        reader.read(file_offset);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(flags);
        writer.write(v_base_addr);
        writer.write(file_offset);
    }
};

/* ModuleType */
CHECK_SIZE(private_fw::ModuleType, 4);
CHECK_MEMBER(private_fw::ModuleType, ul, 0, uint32_t);

union ModuleType
{
    uint32_t ul;
    struct
    {
        uint32_t load_type : 4;  // MT_BUILTIN, MT_LOADABLE
        uint32_t auto_start : 1; // 0 - manually created, 1 - single instance created by
                                 // Module Manager
        uint32_t domain_ll : 1;  // support LL domain
        uint32_t domain_dp : 1;  // support DP domain
        uint32_t _rsvd : 25;
    } r;

    bool operator==(const ModuleType &other) const { return ul == other.ul; }

    void fromStream(util::ByteStreamReader &reader) { reader.read(ul); }

    void toStream(util::ByteStreamWriter &writer) const { writer.write(ul); }
};

/* ModuleEntry */

struct ModuleEntry
{
private:
    template <typename T>
    static bool isArrayEqual(const T *a1, const T *a2, std::size_t size)
    {
        for (std::size_t i = 0; i < size; i++) {
            if (!(a1[i] == a2[i])) {
                return false;
            }
        }
        return true;
    }

public:
    static const std::size_t segmentCount = 3;
    static const std::size_t uuidLen = 4;

    uint16_t module_id;
    uint16_t state_flags;
    uint8_t name[MAX_MODULE_NAME_LEN];
    uint32_t uuid[uuidLen];
    ModuleType type; // ModuleType
    std::array<uint8_t, DEFAULT_HASH_SHA256_LEN> hash;
    uint32_t entry_point;
    uint16_t cfg_offset;
    uint16_t cfg_count;
    uint32_t affinity_mask;       // bit-mask of cores allowed to exec module
    uint16_t instance_max_count;  // max number of instances
    uint16_t instance_stack_size; // size of stack that instance requires for its task
    // (DP) [bytes]
    SegmentDesc segments[segmentCount];

    bool operator==(const ModuleEntry &other) const
    {
        return module_id == other.module_id && state_flags == other.state_flags &&
               isArrayEqual(name, other.name, MAX_MODULE_NAME_LEN) &&
               isArrayEqual(uuid, other.uuid, uuidLen) && type == other.type &&
               hash == other.hash && entry_point == other.entry_point &&
               cfg_offset == other.cfg_offset && cfg_count == other.cfg_count &&
               affinity_mask == other.affinity_mask &&
               instance_max_count == other.instance_max_count &&
               instance_stack_size == other.instance_stack_size &&
               isArrayEqual(segments, other.segments, segmentCount);
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(module_id);
        reader.read(state_flags);
        reader.read(name);
        reader.read(uuid);
        reader.read(type);
        reader.read(hash);
        reader.read(entry_point);
        reader.read(cfg_offset);
        reader.read(cfg_count);
        reader.read(affinity_mask);
        reader.read(instance_max_count);
        reader.read(instance_stack_size);

        for (std::size_t i = 0; i < segmentCount; i++) {
            reader.read(segments[i]);
        }
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(module_id);
        writer.write(state_flags);
        writer.write(name);
        writer.write(uuid);
        writer.write(type);
        writer.write(hash);
        writer.write(entry_point);
        writer.write(cfg_offset);
        writer.write(cfg_count);
        writer.write(affinity_mask);
        writer.write(instance_max_count);
        writer.write(instance_stack_size);

        for (std::size_t i = 0; i < segmentCount; i++) {
            writer.write(segments[i]);
        }
    }

    std::string getName() const
    {
        return util::StringHelper::getStringFromFixedSizeArray(name, sizeof(name));
    }
};

CHECK_SIZE(private_fw::ModuleEntry, 116);
CHECK_MEMBER(private_fw::ModuleEntry, struct_id, 0, uint32_t);
CHECK_MEMBER(private_fw::ModuleEntry, name, 4, uint8_t[MAX_MODULE_NAME_LEN]);
CHECK_MEMBER(private_fw::ModuleEntry, uuid, 12, uint32_t[ModuleEntry::uuidLen]);
CHECK_MEMBER(private_fw::ModuleEntry, type, 28, private_fw::ModuleType);
CHECK_MEMBER(private_fw::ModuleEntry, hash, 32, uint8_t[DEFAULT_HASH_SHA256_LEN]);
CHECK_MEMBER(private_fw::ModuleEntry, entry_point, 64, uint32_t);
CHECK_MEMBER(private_fw::ModuleEntry, cfg_offset, 68, uint16_t);
CHECK_MEMBER(private_fw::ModuleEntry, cfg_count, 70, uint16_t);
CHECK_MEMBER(private_fw::ModuleEntry, affinity_mask, 72, uint32_t);
CHECK_MEMBER(private_fw::ModuleEntry, instance_max_count, 76, uint16_t);
CHECK_MEMBER(private_fw::ModuleEntry, instance_stack_size, 78, uint16_t);
CHECK_MEMBER(private_fw::ModuleEntry, segments, 80,
             private_fw::SegmentDesc[ModuleEntry::segmentCount]);

/* ModulesInfo */

CHECK_SIZE(private_fw::ModulesInfo, 120);
CHECK_MEMBER(private_fw::ModulesInfo, module_count, 0, uint32_t);
CHECK_MEMBER(private_fw::ModulesInfo, module_info, 4, private_fw::ModuleEntry[1]);

struct ModulesInfo
{
    std::vector<ModuleEntry> module_info;

    static std::size_t getAllocationSize(std::size_t count)
    {
        return sizeof(ArraySizeType) + count * sizeof(ModuleEntry);
    }

    bool operator==(const ModulesInfo &other) { return module_info == other.module_info; }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVector<ArraySizeType>(module_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVector<ArraySizeType>(module_info);
    }
};
}
}
}
