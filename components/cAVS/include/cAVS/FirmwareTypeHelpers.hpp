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
#include <string>

namespace debug_agent
{
namespace cavs
{

class FirmwareTypeHelpers
{
public:
    static std::string toString(const dsp_fw::BitDepth &bd)
    {
        int32_t intValue = static_cast<int32_t>(bd);

        switch (bd) {
        case dsp_fw::DEPTH_8BIT:
        case dsp_fw::DEPTH_16BIT:
        case dsp_fw::DEPTH_24BIT:
        case dsp_fw::DEPTH_32BIT:
        case dsp_fw::DEPTH_64BIT:
            /* Enum value is bit depth */
            return std::to_string(intValue);
        }
        return "Unknown bit depth: " + std::to_string(intValue);
    }

    static std::string toString(const dsp_fw::ChannelConfig &cc)
    {
        switch (cc) {
        case dsp_fw::CHANNEL_CONFIG_MONO:
            return "mono";
        case dsp_fw::CHANNEL_CONFIG_STEREO:
            return "stereo";
        case dsp_fw::CHANNEL_CONFIG_2_POINT_1:
            return "2.1";
        case dsp_fw::CHANNEL_CONFIG_3_POINT_0:
            return "3.0";
        case dsp_fw::CHANNEL_CONFIG_3_POINT_1:
            return "3.1";
        case dsp_fw::CHANNEL_CONFIG_QUATRO:
            return "quatro";
        case dsp_fw::CHANNEL_CONFIG_4_POINT_0:
            return "4.0";
        case dsp_fw::CHANNEL_CONFIG_5_POINT_0:
            return "5.0";
        case dsp_fw::CHANNEL_CONFIG_5_POINT_1:
            return "5.1";
        case dsp_fw::CHANNEL_CONFIG_DUAL_MONO:
            return "dual mono";
        case dsp_fw::CHANNEL_CONFIG_I2S_DUAL_STEREO_0:
            return "i2s dual stereo 0";
        case dsp_fw::CHANNEL_CONFIG_I2S_DUAL_STEREO_1:
            return "i2s dual stereo 1";
        case dsp_fw::CHANNEL_CONFIG_7_POINT_1:
            return "7.1";
        }
        return "Unknown channel map: " + std::to_string(static_cast<int32_t>(cc));
    }

    static std::string toString(const dsp_fw::InterleavingStyle &is)
    {
        switch (is) {
        case dsp_fw::CHANNELS_SAMPLES_INTERLEAVING:
            return "sample";
        case dsp_fw::CHANNELS_BLOCKS_INTERLEAVING:
            return "block";
        }
        return "Unknown interleaving style: " + std::to_string(static_cast<int32_t>(is));
    }

    static std::string toString(const dsp_fw::SampleType &st)
    {
        switch (st) {
        case dsp_fw::MSB_INTEGER:
            return "msb integer";
        case dsp_fw::LSB_INTEGER:
            return "lsb integer";
        case dsp_fw::SIGNED_INTEGER:
            return "signed integer";
        case dsp_fw::UNSIGNED_INTEGER:
            return "unsigned integer";
        case dsp_fw::FLOAT:
            return "float";
        }
        return "Unknown sample type: " + std::to_string(static_cast<int32_t>(st));
    }

    static std::string toString(const dsp_fw::AudioDataFormatIpc &format)
    {
        return "config=" + toString(format.channel_config) +
            "/bit_depth=" + toString(format.bit_depth) +
            "/sample_type=" + toString(format.sample_type) +
            "/interleaving=" + toString(format.interleaving_style) +
            "/channel_count=" + std::to_string(format.number_of_channels) +
            "/valid_bit_depth=" + std::to_string(format.valid_bit_depth);
    }
private:
    FirmwareTypeHelpers();
};

}
}
