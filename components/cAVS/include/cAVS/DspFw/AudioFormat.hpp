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
#include "Util/EnumHelper.hpp"
#include "Util/StructureChangeTracking.hpp"
#include <inttypes.h>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/* Importing StreamType enum */
using StreamType = private_fw::StreamType;
static_assert(StreamType::eMaxStreamType == 3, "Wrong StreamType enum value count");

static const util::EnumHelper<StreamType> &getStreamTypeHelper()
{
    static const util::EnumHelper<StreamType> helper({
        {StreamType::ePcm, "PCM"}, {StreamType::eMp3, "MP3"}, {StreamType::eAac, "AAC"},
    });

    return helper;
};

/* Importing SamplingFrequency enum */
using SamplingFrequency = private_fw::intel_adsp::SamplingFrequency;

/* Importing BitDepth enum */
using BitDepth = private_fw::intel_adsp::BitDepth;

/* Importing InterleavingStyle enum */
using InterleavingStyle = private_fw::intel_adsp::InterleavingStyle;

static const util::EnumHelper<InterleavingStyle> &getInterleavingStyleHelper()
{
    static const util::EnumHelper<InterleavingStyle> helper({
        {InterleavingStyle::CHANNELS_SAMPLES_INTERLEAVING, "sample"},
        {InterleavingStyle::CHANNELS_BLOCKS_INTERLEAVING, "block"},
    });

    return helper;
};

/* Importing ChannelMap */
using ChannelMap = private_fw::intel_adsp::ChannelMap;

/* Importing ChannelConfig enum */
using ChannelConfig = private_fw::intel_adsp::ChannelConfig;
static_assert(ChannelConfig::CHANNEL_CONFIG_INVALID == 13, "Wrong ChannelConfig enum value count");

static const util::EnumHelper<ChannelConfig> &getChannelConfigHelper()
{
    static const util::EnumHelper<ChannelConfig> helper({
        {ChannelConfig::CHANNEL_CONFIG_MONO, "mono"},
        {ChannelConfig::CHANNEL_CONFIG_STEREO, "stereo"},
        {ChannelConfig::CHANNEL_CONFIG_2_POINT_1, "2.1"},
        {ChannelConfig::CHANNEL_CONFIG_3_POINT_0, "3.0"},
        {ChannelConfig::CHANNEL_CONFIG_3_POINT_1, "301"},
        {ChannelConfig::CHANNEL_CONFIG_QUATRO, "quatro"},
        {ChannelConfig::CHANNEL_CONFIG_4_POINT_0, "4.0"},
        {ChannelConfig::CHANNEL_CONFIG_5_POINT_0, "5.0"},
        {ChannelConfig::CHANNEL_CONFIG_5_POINT_1, "5.1"},
        {ChannelConfig::CHANNEL_CONFIG_DUAL_MONO, "dual mono"},
        {ChannelConfig::CHANNEL_CONFIG_I2S_DUAL_STEREO_0, "i2s dual stereo 0"},
        {ChannelConfig::CHANNEL_CONFIG_I2S_DUAL_STEREO_1, "i2s dual stereo 1"},
        {ChannelConfig::CHANNEL_CONFIG_7_POINT_1, "7.1"},
    });
    return helper;
};

/* Importing SampleType enum */
using SampleType = private_fw::intel_adsp::SampleType;
static const util::EnumHelper<SampleType> &getSampleTypeHelper()
{
    static const util::EnumHelper<SampleType> helper({
        {SampleType::MSB_INTEGER, "msb integer"},
        {SampleType::LSB_INTEGER, "lsb integer"},
        {SampleType::SIGNED_INTEGER, "signed integer"},
        {SampleType::UNSIGNED_INTEGER, "unsigned integer"},
        {SampleType::FLOAT, "float"},
    });

    return helper;
};

/* AudioDataFormatIpc */

CHECK_SIZE(private_fw::dsp_fw::AudioDataFormatIpc, 24);
CHECK_MEMBER(private_fw::dsp_fw::AudioDataFormatIpc, sampling_frequency, 0, SamplingFrequency);
CHECK_MEMBER(private_fw::dsp_fw::AudioDataFormatIpc, bit_depth, 4, BitDepth);
CHECK_MEMBER(private_fw::dsp_fw::AudioDataFormatIpc, channel_map, 8, ChannelMap);
CHECK_MEMBER(private_fw::dsp_fw::AudioDataFormatIpc, channel_config, 12, ChannelConfig);
CHECK_MEMBER(private_fw::dsp_fw::AudioDataFormatIpc, interleaving_style, 16, InterleavingStyle);
/* The following members can not be checked because they are bitfields
* - number_of_channels
* - valid_bit_depth
* - sample_type
* - reserved
*/
struct AudioDataFormatIpc
{
    SamplingFrequency sampling_frequency; /*!< Sampling frequency in Hz */
    BitDepth bit_depth;                   /*!< Bit depth of audio samples */
    ChannelMap channel_map;               /*!< channel ordering in audio stream */
    ChannelConfig channel_config;         /*!< Channel configuration. */
    InterleavingStyle interleaving_style; /*!< The way the samples are interleaved */
    uint8_t number_of_channels;           /*!< Total number of channels. */
    uint8_t valid_bit_depth;              /*!< Valid bit depth in audio samples */
    SampleType sample_type : 8;           /*!< sample type:
                                            *  0 - intMSB
                                            *  1 - intLSB
                                            *  2 - intSinged
                                            *  3 - intUnsigned
                                            *  4 - float  */
    uint8_t reserved;                     /*!< padding byte */

    bool operator==(const AudioDataFormatIpc &other) const
    {
        return bit_depth == other.bit_depth && channel_config == other.channel_config &&
               channel_map == other.channel_map && interleaving_style == other.interleaving_style &&
               number_of_channels == other.number_of_channels && reserved == other.reserved &&
               sample_type == other.sample_type && sampling_frequency == other.sampling_frequency &&
               valid_bit_depth == other.valid_bit_depth;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(sampling_frequency);
        reader.read(bit_depth);
        reader.read(channel_map);
        reader.read(channel_config);
        reader.read(interleaving_style);
        reader.read(number_of_channels);
        reader.read(valid_bit_depth);

        /* sample_type is a enum:8 type */
        uint8_t sample_type_as_byte;
        reader.read(sample_type_as_byte);
        sample_type = static_cast<SampleType>(sample_type_as_byte);

        reader.read(reserved);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(sampling_frequency);
        writer.write(bit_depth);
        writer.write(channel_map);
        writer.write(channel_config);
        writer.write(interleaving_style);
        writer.write(number_of_channels);
        writer.write(valid_bit_depth);

        /* sample_type_as_byte is a enum:8 type */
        writer.write<uint8_t>(sample_type);

        writer.write(reserved);
    }

    std::string toString() const
    {
        return "config=" + getChannelConfigHelper().toString(channel_config) + "/bit_depth=" +
               std::to_string(bit_depth) + "/sample_type=" +
               getSampleTypeHelper().toString(sample_type) + "/interleaving=" +
               getInterleavingStyleHelper().toString(interleaving_style) + "/channel_count=" +
               std::to_string(number_of_channels) + "/valid_bit_depth=" +
               std::to_string(valid_bit_depth);
    }
};
}
}
}
