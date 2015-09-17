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

#include <inttypes.h>

namespace debug_agent
{
namespace cavs
{

/** This header already defines the dsp_fw namespace, so including it into debug_agent::cavs ...*/
#include "basefw_config.h"

/** Including the firmware types into the debug_agent::cavs::dsp_fw namespace */
namespace dsp_fw
{
#include "adsp_ixc_status.h"

/** This type does not exist in the fw yet */
enum StreamType
{
    STREAM_TYPE_PCM = 0,
    STREAM_TYPE_MP3 = 1
};

/* This type exists in the fw header but is not easily includable, so copying it */
enum SamplingFrequency
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
    FS_48000HZ = 48000, /**< Default. */
    FS_64000HZ = 64000, /** AAC, SRC only. */
    FS_88200HZ = 88200, /** AAC, SRC only. */
    FS_96000HZ = 96000, /** AAC, SRC only. */
    FS_176400HZ = 176400, /** SRC only. */
    FS_192000HZ = 192000, /** SRC only. */
    FS_INVALID
};

/* This type exists in the fw header but is not easily includable, so copying it */
enum BitDepth
{
    DEPTH_8BIT = 8,
    DEPTH_16BIT = 16,
    DEPTH_24BIT = 24, /**< Default. */
    DEPTH_32BIT = 32,
    DEPTH_64BIT = 64,
    DEPTH_INVALID
};

/* This type exists in the fw header but is not easily includable, so copying it */
enum InterleavingStyle
{
    CHANNELS_SAMPLES_INTERLEAVING = 0, /*!< [s1_ch1...s1_chN,...,sM_ch1...sM_chN] */
    CHANNELS_BLOCKS_INTERLEAVING = 1, /*!< [s1_ch1...sM_ch1,...,s1_chN...sM_chN] */
};

/* This type exists in the fw header but is not easily includable, so copying it */
typedef uint32_t ChannelMap;

/* This type exists in the fw header but is not easily includable, so copying it */
enum ChannelConfig
{
    CHANNEL_CONFIG_MONO = 0, /**< One channel only. */
    CHANNEL_CONFIG_STEREO = 1, /**< L & R. */
    CHANNEL_CONFIG_2_POINT_1 = 2, /**< L, R & LFE; PCM only. */
    CHANNEL_CONFIG_3_POINT_0 = 3, /**< L, C & R; MP3 & AAC only. */
    CHANNEL_CONFIG_3_POINT_1 = 4, /**< L, C, R & LFE; PCM only. */
    CHANNEL_CONFIG_QUATRO = 5, /**< L, R, Ls & Rs; PCM only. */
    CHANNEL_CONFIG_4_POINT_0 = 6, /**< L, C, R & Cs; MP3 & AAC only. */
    CHANNEL_CONFIG_5_POINT_0 = 7, /**< L, C, R, Ls & Rs. */
    CHANNEL_CONFIG_5_POINT_1 = 8, /**< L, C, R, Ls, Rs & LFE. */
    CHANNEL_CONFIG_DUAL_MONO = 9, /**< One channel replicated in two. */
    CHANNEL_CONFIG_I2S_DUAL_STEREO_0 = 10, /**< Stereo (L,R) in 4 slots, 1st stream: [ L, R, -, - ] */
    CHANNEL_CONFIG_I2S_DUAL_STEREO_1 = 11, /**< Stereo (L,R) in 4 slots, 2nd stream: [ -, -, L, R ] */
    CHANNEL_CONFIG_7_POINT_1 = 12, /**< L, C, R, Ls, Rs & LFE., LS, RS */
    CHANNEL_CONFIG_INVALID
};

/* This type exists in the fw header but is not easily includable, so copying it */
enum SampleType
{
    MSB_INTEGER = 0, /*!< integer with Most Significant Byte first */
    LSB_INTEGER = 1, /*!< integer with Least Significant Byte first */
    SIGNED_INTEGER = 2, /*!< signed integer */
    UNSIGNED_INTEGER = 3, /*!< unsigned integer */
    FLOAT = 4 /*!< unsigned integer */
};

/* This type exists in the fw header but is not easily includable, so copying it */
struct AudioDataFormatIpc
{
    SamplingFrequency sampling_frequency : 32; /*!< Sampling frequency in Hz */
    BitDepth bit_depth : 32; /*!< Bit depth of audio samples */
    ChannelMap channel_map : 32; /*!< channel ordering in audio stream */
    ChannelConfig channel_config : 32; /*!< Channel configuration. */
    InterleavingStyle interleaving_style : 32; /*!< The way the samples are interleaved */
    uint32_t number_of_channels : 8; /*!< Total number of channels. */
    uint32_t valid_bit_depth : 8; /*!< Valid bit depth in audio samples */
    SampleType sample_type : 8; /*!< sample type:
                                *  0 - intMSB
                                *  1 - intLSB
                                *  2 - intSinged
                                *  3 - intUnsigned
                                *  4 - float  */
    uint32_t reserved : 8;  /*!< padding byte */
};

/* This type exists in the fw header but is not easily includable, so copying it */
enum BaseModuleParams
{
    /**
    * Handled inside LargeConfigGet of module instance
    */
    MOD_INST_PROPS = 0xFE,
    /**
    * Handled inside ConfigSet of module instance
    */
    MOD_INST_ENABLE = 0x3000
};

/** This type does not exist in the fw yet */
struct PinProps
{
    StreamType          stream_type;
    AudioDataFormatIpc  format;
    uint32_t            phys_queue_id;

    static const uint32_t invalidQueueId = 0xFFFFFFFF;
};

/* This type exists in the fw header but is not easily includable, so copying it */
struct ConnectorNodeId
{
    union
    {
        uint32_t dw;
        struct
        {
            uint32_t v_index : 8;
            uint32_t dma_type : 5;
            uint32_t _rsvd : 19;
        } f;
    } val;

    // TODO: enum should be splitted into child classes
    enum Type
    {
        kHdaHostOutputClass = 0,
        kHdaHostInputClass = 1,
        kHdaHostInoutClass = 2,   // for future use
        kHdaLinkOutputClass = 8,
        kHdaLinkInputClass = 9,
        kHdaLinkInoutClass = 10,  // for future use
        kDmicLinkInputClass = 11,
        kI2sLinkOutputClass = 12,
        kI2sLinkInputClass = 13,
        kSlimbusLinkOutputClass = 14,
        kSlimbusLinkInputClass = 15,
        kALHLinkOutputClass = 16,
        kALHLinkInputClass = 17
    };

    /**
    * Creates the connector node id from dma type and virtual dma index.
    * @param dma_type Type of DMA Connector.
    * @param v_index Virtual DMA Index.
    */
    ConnectorNodeId(Type dma_type, uint8_t v_index)
    {
        Init(dma_type, v_index);
    }

    explicit ConnectorNodeId(uint32_t node_id)
    {
        val.dw = node_id;
    }
    static const uint32_t kInvalidNodeId = 0xffffffff;

    ConnectorNodeId()
    {
        val.dw = kInvalidNodeId;
    }

    void Init(Type dma_type, uint8_t v_index)
    {
        val.dw = 0;
        val.f.dma_type = dma_type;
        val.f.v_index = v_index;
    }

    inline uint32_t GetBareNodeId() const { return val.dw; }
};

}
}
}
