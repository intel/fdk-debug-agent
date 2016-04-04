/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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

#include "cAVS/DspFw/ExternalFirmwareHeaders.hpp"
#include "Util/Buffer.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/EnumHelper.hpp"
#include "Util/Exception.hpp"
#include "Util/StructureChangeTracking.hpp"
#include "Util/AssertAlways.hpp"
#include <cstdint>
#include <string>
#include <sstream>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

enum class ProbeType
{
    Input = private_fw::dsp_fw::INPUT,
    Output = private_fw::dsp_fw::OUTPUT,
    Internal = private_fw::dsp_fw::INTERNAL
};

static const util::EnumHelper<ProbeType> &probeTypeHelper()
{
    static const util::EnumHelper<ProbeType> helper({
        {ProbeType::Input, "Input"},
        {ProbeType::Output, "Output"},
        {ProbeType::Internal, "Internal"},
    });
    return helper;
}

namespace probe_point_id
{
// Cannot not add these static variables in ProbePointId  because
// ProbePointId is an union
static constexpr int moduleIdSize{16};
static constexpr int instanceIdSize{8};
static constexpr int typeSize{2};
static constexpr int indexSize{6};
}

/** Identify a probe point into the topology */
using private_ProbePointId = private_fw::dsp_fw::ProbePointId;
CHECK_SIZE(private_ProbePointId, 4);
CHECK_MEMBER(private_ProbePointId, full_probe_id, 0, uint32_t);
union ProbePointId
{
    using Exception = util::Exception<ProbePointId>;

    struct
    {
    public:
        /**@{
         * Using setters to validate values
         * @throw ProbePointId::Exception if value is invalid
         */
        void setModuleId(uint32_t moduleId)
        {
            if (moduleId >= 1 << probe_point_id::moduleIdSize) {
                throw Exception("Module id too large (" + std::to_string(moduleId) + ")");
            }
            this->moduleId = moduleId;
        }

        void setInstanceId(uint32_t instanceId)
        {
            if (instanceId >= 1 << probe_point_id::instanceIdSize) {
                throw Exception("Instance id too large (" + std::to_string(instanceId) + ")");
            }
            this->instanceId = instanceId;
        }

        void setType(ProbeType type)
        {
            auto typeAsInt = static_cast<uint32_t>(type);
            if (!probeTypeHelper().isValid(type)) {
                throw Exception("Invalid probe type (" + std::to_string(typeAsInt) + ")");
            }
            this->type = static_cast<uint32_t>(type);
        }

        void setIndex(uint32_t index)
        {
            if (index >= 1 << probe_point_id::indexSize) {
                throw Exception("Pin index too large (" + std::to_string(index) + ")");
            }
            this->index = index;
        }

        /**@}*/

        uint32_t getModuleId() const { return moduleId; }
        uint32_t getInstanceId() const { return instanceId; }
        ProbeType getType() const { return static_cast<ProbeType>(type); }
        uint32_t getIndex() const { return index; }

    private:
        uint32_t moduleId : probe_point_id::moduleIdSize;
        uint32_t instanceId : probe_point_id::instanceIdSize;

        // Can not use directly ProbeType here because gcc complains "type  is too small to hold
        // all values"
        uint32_t type : probe_point_id::typeSize;

        uint32_t index : probe_point_id::indexSize;
    } fields;
    uint32_t full;

    ProbePointId() = default;

    /** @throw ProbePointId::Exception if values are not in valid range */
    ProbePointId(uint32_t moduleId, uint32_t instanceId, ProbeType type, uint32_t index)
    {
        fields.setModuleId(moduleId);
        fields.setInstanceId(instanceId);
        fields.setType(type);
        fields.setIndex(index);
    }

    std::string toString() const
    {
        std::stringstream stream;
        stream << "moduleId=" << fields.getModuleId() << " instanceId=" << fields.getInstanceId()
               << " type=" << probeTypeHelper().toString(fields.getType())
               << " index=" << fields.getIndex();
        return stream.str();
    }

    bool operator==(const ProbePointId &other) const { return full == other.full; }

    /** Operator< enables using this type as key map*/
    bool operator<(const ProbePointId &other) const { return full < other.full; }

    void fromStream(util::ByteStreamReader &reader) { reader.read(full); }

    void toStream(util::ByteStreamWriter &writer) const { writer.write(full); }
};

/** Probe packet
 * @todo Currently it is not possible to track changes of the original firmware structure
 * because the inclusion of the <ixc/probe_header_defs.h> header produces this error:
 *    private_fw::intel_adsp::InterleavingStyle' : redefinition; different basic types
 */
struct Packet
{
    Packet() = default;
    Packet(Packet &&) = default;
    Packet &operator=(Packet &&) = default;

    /** Probe packet sync word. TODO: Should be include from fw headers. */
    static constexpr const uint32_t syncWord = 0xBABEBEBA;

    ProbePointId probePointId;
    uint32_t format;
    uint32_t dspWallClockTsHw;
    uint32_t dspWallClockTsLw;
    util::Buffer data;
    uint32_t headerChecksum;

    uint32_t sum() const
    {
        return syncWord + probePointId.full + format + dspWallClockTsHw + dspWallClockTsLw +
               static_cast<uint32_t>(data.size());
    }

    std::string toString() const
    {
        using std::to_string;
        std::stringstream ss;
        ss << "Probe packet header { probe point id {" << probePointId.toString() << "} "
           << std::hex << std::showbase << "syncWord=" << syncWord << ", format=" << format
           << ", dspWallClockTsHw=" << dspWallClockTsHw << ", dspWallClockTsLw=" << dspWallClockTsLw
           << ", dataSize=" << data.size() << " }";
        return ss.str();
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        uint32_t syncWordValue;
        reader.read(syncWordValue);

        if (syncWordValue != syncWord) {
            throw util::ByteStreamReader::Exception(
                "Invalid sync word in extracted probe packet header. "
                "Expected " +
                std::to_string(syncWord) + ", found " + std::to_string(syncWordValue));
        }

        reader.read(probePointId);
        reader.read(format);
        reader.read(dspWallClockTsHw);
        reader.read(dspWallClockTsLw);
        reader.readVector<uint32_t>(data);

        uint32_t headerChecksumValue;
        reader.read(headerChecksumValue);
        if (headerChecksumValue != sum()) {
            throw util::ByteStreamReader::Exception("Header checksum mismatch. Expected " +
                                                    std::to_string(sum()) + ", found " +
                                                    std::to_string(headerChecksumValue) +
                                                    ". While checking integrity of " + toString());
        }
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(syncWord);
        writer.write(probePointId);
        writer.write(format);
        writer.write(dspWallClockTsHw);
        writer.write(dspWallClockTsLw);
        writer.writeVector<uint32_t>(data);
        writer.write(sum());
    }
};
}
}
}
