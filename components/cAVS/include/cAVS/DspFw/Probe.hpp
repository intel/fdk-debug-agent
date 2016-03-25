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

#include "Util/Buffer.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include <cstdint>
#include <string>
#include <sstream>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/** Probe packet. TODO: Should be include from fw headers. */
struct Packet
{
    Packet() = default;
    Packet(Packet &&) = default;
    Packet &operator=(Packet &&) = default;

    /** Probe packet sync word. TODO: Should be include from fw headers. */
    static constexpr const uint32_t syncWord = 0xBABEBEBA;

    uint32_t probePointId;
    uint32_t format;
    uint32_t dspWallClockTsHw;
    uint32_t dspWallClockTsLw;
    util::Buffer data;
    uint32_t headerChecksum;

    uint32_t sum() const
    {
        return syncWord + probePointId + format + dspWallClockTsHw + dspWallClockTsLw +
               static_cast<uint32_t>(data.size());
    }

    std::string toString() const
    {
        using std::to_string;
        std::stringstream ss;
        ss << "Probe packet header { " << std::hex << std::showbase << "syncWord=" << syncWord
           << ", format=" << format << ", dspWallClockTsHw=" << dspWallClockTsHw
           << ", dspWallClockTsLw=" << dspWallClockTsLw << ", dataSize=" << data.size() << " }";
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
