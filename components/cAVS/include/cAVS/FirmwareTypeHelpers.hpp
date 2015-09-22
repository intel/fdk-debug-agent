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

#include "cAVS/FirmwareTypes.hpp"
#include "Util/EnumHelper.hpp"
#include <string>

namespace debug_agent
{
namespace cavs
{

class FirmwareTypeHelpers
{
public:
    /* Enumeration helpers */
    /* ------------------- */

    static const util::EnumHelper<dsp_fw::BitDepth> &getBitDepthHelper()
    {
        static const util::EnumHelper<dsp_fw::BitDepth> helper({
            { dsp_fw::BitDepth::DEPTH_8BIT, "8" },
            { dsp_fw::BitDepth::DEPTH_16BIT, "16" },
            { dsp_fw::BitDepth::DEPTH_24BIT, "24" },
            { dsp_fw::BitDepth::DEPTH_32BIT, "32" },
            { dsp_fw::BitDepth::DEPTH_64BIT, "64" },
        });
        return helper;
    };

    static const util::EnumHelper<dsp_fw::ChannelConfig> &getChannelConfigHelper()
    {
        static const util::EnumHelper<dsp_fw::ChannelConfig> helper({
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_MONO, "mono" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_STEREO, "stereo" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_2_POINT_1, "2.1" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_3_POINT_0, "3.0" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_3_POINT_1, "301" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_QUATRO, "quatro" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_4_POINT_0, "4.0" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_5_POINT_0, "5.0" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_5_POINT_1, "5.1" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_DUAL_MONO, "dual mono" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_I2S_DUAL_STEREO_0, "i2s dual stereo 0" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_I2S_DUAL_STEREO_1, "i2s dual stereo 1" },
            { dsp_fw::ChannelConfig::CHANNEL_CONFIG_7_POINT_1, "7.1" },
        });
        return helper;
    };

    static const util::EnumHelper<dsp_fw::InterleavingStyle> &getInterleavingStyleHelper()
    {
        static const util::EnumHelper<dsp_fw::InterleavingStyle> helper({
            { dsp_fw::InterleavingStyle::CHANNELS_SAMPLES_INTERLEAVING, "sample" },
            { dsp_fw::InterleavingStyle::CHANNELS_BLOCKS_INTERLEAVING, "block" },
        });

        return helper;
    };

    static const util::EnumHelper<dsp_fw::SampleType> &getSampleTypeHelper()
    {
        static const util::EnumHelper<dsp_fw::SampleType> helper({
            { dsp_fw::SampleType::MSB_INTEGER, "msb integer" },
            { dsp_fw::SampleType::LSB_INTEGER, "lsb integer" },
            { dsp_fw::SampleType::SIGNED_INTEGER, "signed integer" },
            { dsp_fw::SampleType::UNSIGNED_INTEGER, "unsigned integer" },
            { dsp_fw::SampleType::FLOAT, "float" },
        });

        return helper;
    };

    static const util::EnumHelper<dsp_fw::ConnectorNodeId::Type> &getGatewayHelper()
    {
        static const util::EnumHelper<dsp_fw::ConnectorNodeId::Type> helper({
            { dsp_fw::ConnectorNodeId::kHdaHostOutputClass, "hda-host-out-gateway" },
            { dsp_fw::ConnectorNodeId::kHdaHostInputClass, "hda-host-in-gateway" },
            { dsp_fw::ConnectorNodeId::kHdaHostInoutClass, "hda-host-inout-gateway" },
            { dsp_fw::ConnectorNodeId::kHdaLinkOutputClass, "hda-link-out-gateway" },
            { dsp_fw::ConnectorNodeId::kHdaLinkInputClass, "hda-link-in-gateway" },
            { dsp_fw::ConnectorNodeId::kHdaLinkInoutClass, "hda-link-inout-gateway" },
            { dsp_fw::ConnectorNodeId::kDmicLinkInputClass, "dmic-link-in-gateway" },
            { dsp_fw::ConnectorNodeId::kI2sLinkOutputClass, "i2s-link-out-gateway" },
            { dsp_fw::ConnectorNodeId::kI2sLinkInputClass, "i2s-link-in-gateway" },
            { dsp_fw::ConnectorNodeId::kSlimbusLinkOutputClass, "slimbus-link-out-gateway" },
            { dsp_fw::ConnectorNodeId::kSlimbusLinkInputClass, "slimbus-link-in-gateway" },
            { dsp_fw::ConnectorNodeId::kALHLinkOutputClass, "alh-link-out-gateway" },
            { dsp_fw::ConnectorNodeId::kALHLinkInputClass, "alh-link-in-gateway" }
        });

        return helper;
    };

    static const util::EnumHelper<dsp_fw::StreamType> &getStreamTypeHelper()
    {
        static const util::EnumHelper<dsp_fw::StreamType> helper({
            { dsp_fw::StreamType::STREAM_TYPE_PCM, "PCM" },
            { dsp_fw::StreamType::STREAM_TYPE_MP3, "MP3" },
        });

        return helper;
    };

    /* Firmware types to string */
    /* ------------------------ */

    static std::string toString(const dsp_fw::AudioDataFormatIpc &format)
    {
        return "config=" + getChannelConfigHelper().toString(format.channel_config) +
            "/bit_depth=" + getBitDepthHelper().toString(format.bit_depth) +
            "/sample_type=" + getSampleTypeHelper().toString(format.sample_type) +
            "/interleaving=" + getInterleavingStyleHelper().toString(format.interleaving_style) +
            "/channel_count=" + std::to_string(format.number_of_channels) +
            "/valid_bit_depth=" + std::to_string(format.valid_bit_depth);
    }

private:
    FirmwareTypeHelpers();
};

}
}
