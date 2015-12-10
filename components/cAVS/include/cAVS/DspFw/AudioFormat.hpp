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
#include "Util/EnumHelper.hpp"
#include <inttypes.h>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

enum class StreamType : uint32_t
{
    STREAM_TYPE_PCM = 0,
    STREAM_TYPE_MP3 = 1
};

static const util::EnumHelper<StreamType> &getStreamTypeHelper()
{
    static const util::EnumHelper<StreamType> helper({
        {StreamType::STREAM_TYPE_PCM, "PCM"}, {StreamType::STREAM_TYPE_MP3, "MP3"},
    });

    return helper;
};

enum class SamplingFrequency : uint32_t
{
    FS_8000HZ = 8000,
    FS_11025HZ = 11025,
    FS_12000HZ = 12000, /** Mp3, AAC, SRC only. */
    FS_16000HZ = 16000,
    FS_18900HZ = 18900, /** SRC only for 44100 */
    FS_22050HZ = 22050,
    FS_24000HZ = 24000, /** Mp3, AAC, SRC only. */
    FS_32000HZ = 32000,
    FS_37800HZ = 37800, /** SRC only for 44100 */
    FS_44100HZ = 44100,
    FS_48000HZ = 48000,   /**< Default. */
    FS_64000HZ = 64000,   /** AAC, SRC only. */
    FS_88200HZ = 88200,   /** AAC, SRC only. */
    FS_96000HZ = 96000,   /** AAC, SRC only. */
    FS_176400HZ = 176400, /** SRC only. */
    FS_192000HZ = 192000, /** SRC only. */
    FS_INVALID
};

enum class BitDepth : uint32_t
{
    DEPTH_8BIT = 8,
    DEPTH_16BIT = 16,
    DEPTH_24BIT = 24, /**< Default. */
    DEPTH_32BIT = 32,
    DEPTH_64BIT = 64,
    DEPTH_INVALID
};

static const util::EnumHelper<BitDepth> &getBitDepthHelper()
{
    static const util::EnumHelper<BitDepth> helper({
        {BitDepth::DEPTH_8BIT, "8"},
        {BitDepth::DEPTH_16BIT, "16"},
        {BitDepth::DEPTH_24BIT, "24"},
        {BitDepth::DEPTH_32BIT, "32"},
        {BitDepth::DEPTH_64BIT, "64"},
    });
    return helper;
};

enum class InterleavingStyle : uint32_t
{
    CHANNELS_SAMPLES_INTERLEAVING = 0, /*!< [s1_ch1...s1_chN,...,sM_ch1...sM_chN] */
    CHANNELS_BLOCKS_INTERLEAVING = 1,  /*!< [s1_ch1...sM_ch1,...,s1_chN...sM_chN] */
};

static const util::EnumHelper<InterleavingStyle> &getInterleavingStyleHelper()
{
    static const util::EnumHelper<InterleavingStyle> helper({
        {InterleavingStyle::CHANNELS_SAMPLES_INTERLEAVING, "sample"},
        {InterleavingStyle::CHANNELS_BLOCKS_INTERLEAVING, "block"},
    });

    return helper;
};

using ChannelMap = uint32_t;

enum class ChannelConfig : uint32_t
{
    CHANNEL_CONFIG_MONO = 0,               /**< One channel only. */
    CHANNEL_CONFIG_STEREO = 1,             /**< L & R. */
    CHANNEL_CONFIG_2_POINT_1 = 2,          /**< L, R & LFE; PCM only. */
    CHANNEL_CONFIG_3_POINT_0 = 3,          /**< L, C & R; MP3 & AAC only. */
    CHANNEL_CONFIG_3_POINT_1 = 4,          /**< L, C, R & LFE; PCM only. */
    CHANNEL_CONFIG_QUATRO = 5,             /**< L, R, Ls & Rs; PCM only. */
    CHANNEL_CONFIG_4_POINT_0 = 6,          /**< L, C, R & Cs; MP3 & AAC only. */
    CHANNEL_CONFIG_5_POINT_0 = 7,          /**< L, C, R, Ls & Rs. */
    CHANNEL_CONFIG_5_POINT_1 = 8,          /**< L, C, R, Ls, Rs & LFE. */
    CHANNEL_CONFIG_DUAL_MONO = 9,          /**< One channel replicated in two. */
    CHANNEL_CONFIG_I2S_DUAL_STEREO_0 = 10, /**< Stereo (L,R) in 4 slots, 1st stream:
                                            * [ L, R, -, - ] */
    CHANNEL_CONFIG_I2S_DUAL_STEREO_1 = 11, /**< Stereo (L,R) in 4 slots, 2nd stream:
                                            * [ -, -, L, R ] */
    CHANNEL_CONFIG_7_POINT_1 = 12,         /**< L, C, R, Ls, Rs & LFE., LS, RS */
    CHANNEL_CONFIG_INVALID
};

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

enum class SampleType : uint8_t
{
    MSB_INTEGER = 0,      /*!< integer with Most Significant Byte first */
    LSB_INTEGER = 1,      /*!< integer with Least Significant Byte first */
    SIGNED_INTEGER = 2,   /*!< signed integer */
    UNSIGNED_INTEGER = 3, /*!< unsigned integer */
    FLOAT = 4             /*!< unsigned integer */
};

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

struct AudioDataFormatIpc
{
    SamplingFrequency sampling_frequency; /*!< Sampling frequency in Hz */
    BitDepth bit_depth;                   /*!< Bit depth of audio samples */
    ChannelMap channel_map;               /*!< channel ordering in audio stream */
    ChannelConfig channel_config;         /*!< Channel configuration. */
    InterleavingStyle interleaving_style; /*!< The way the samples are interleaved */
    uint8_t number_of_channels;           /*!< Total number of channels. */
    uint8_t valid_bit_depth;              /*!< Valid bit depth in audio samples */
    SampleType sample_type;               /*!< sample type:
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
        reader.read(sample_type);
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
        writer.write(sample_type);
        writer.write(reserved);
    }

    std::string toString() const
    {
        return "config=" + getChannelConfigHelper().toString(channel_config) + "/bit_depth=" +
               getBitDepthHelper().toString(bit_depth) + "/sample_type=" +
               getSampleTypeHelper().toString(sample_type) + "/interleaving=" +
               getInterleavingStyleHelper().toString(interleaving_style) + "/channel_count=" +
               std::to_string(number_of_channels) + "/valid_bit_depth=" +
               std::to_string(valid_bit_depth);
    }
};
static_assert(sizeof(AudioDataFormatIpc) == 24, "Wrong AudioDataFormatIpc size");
}
}
}
