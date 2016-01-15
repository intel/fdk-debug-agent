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

#include "cAVS/DspFw/ExternalFirmwareHeaders.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/StructureChangeTracking.hpp"

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* FwVersion */

CHECK_SIZE(private_fw::FwVersion, 8);
CHECK_MEMBER(private_fw::FwVersion, major, 0, uint16_t);
CHECK_MEMBER(private_fw::FwVersion, minor, 2, uint16_t);
CHECK_MEMBER(private_fw::FwVersion, hotfix, 4, uint16_t);
CHECK_MEMBER(private_fw::FwVersion, build, 6, uint16_t);

struct FwVersion
{
    uint16_t major;
    uint16_t minor;
    uint16_t hotfix;
    uint16_t build;

    bool operator==(const FwVersion &other)
    {
        return major == other.major && minor == other.minor && hotfix == other.hotfix &&
               build == other.build;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(major);
        reader.read(minor);
        reader.read(hotfix);
        reader.read(build);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(major);
        writer.write(minor);
        writer.write(hotfix);
        writer.write(build);
    }
};

/* DmaBufferConfig */

CHECK_SIZE(private_fw::DmaBufferConfig, 8);
CHECK_MEMBER(private_fw::DmaBufferConfig, min_size_bytes, 0, uint32_t);
CHECK_MEMBER(private_fw::DmaBufferConfig, max_size_bytes, 4, uint32_t);

struct DmaBufferConfig
{
    uint32_t min_size_bytes;
    uint32_t max_size_bytes;

    bool operator==(const DmaBufferConfig &other)
    {
        return min_size_bytes == other.min_size_bytes && max_size_bytes == other.max_size_bytes;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(min_size_bytes);
        reader.read(max_size_bytes);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(min_size_bytes);
        writer.write(max_size_bytes);
    }
};

/* Importing FwConfigParams enum */
using FwConfigParams = private_fw::FwConfigParams;

/* Importing HwConfigParams enum */
using HwConfigParams = private_fw::HwConfigParams;
}
}
}
