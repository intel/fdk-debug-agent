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
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"

/* IMPORTANT: this header is a mess and will be split in a subsequent patch. */

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

static const uint32_t IXC_STATUS_BITS = 24;

enum class IxcStatus
{
    ADSP_IPC_SUCCESS = 0,                       ///< The operation was successful.

    ADSP_IPC_ERROR_INVALID_PARAM = 1,           ///< Invalid parameter was passed.
    ADSP_IPC_UNKNOWN_MESSAGE_TYPE = 2,          ///< Uknown message type was received.

    ADSP_IPC_OUT_OF_MEMORY = 3,                 ///< No physical memory to satisfy the request.

    ADSP_IPC_BUSY = 4,                          ///< The system or resource is busy.
    ADSP_IPC_PENDING = 5,                       ///< The action was scheduled for processing.
    ADSP_IPC_FAILURE = 6,                       ///< Critical error happened.
    ADSP_IPC_INVALID_REQUEST = 7,               ///< Request can not be completed.

    ADSP_RSVD_8 = 8,                            ///< Formerly ADSP_STAGE_UNINITIALIZED.

    ADSP_IPC_INVALID_RESOURCE_ID = 9,           ///< Required resource can not be found.

    ADSP_RSVD_10 = 10,                          ///< Formerly ADSP_SOURCE_NOT_STARTED.

    ASDP_IPC_OUT_OF_MIPS = 11,                  ///< No computation power to satisfy the request.

    ADSP_IPC_INVALID_RESOURCE_STATE = 12,       ///< Required resource is in invalid state.

    ADSP_IPC_POWER_TRANSITION_FAILED = 13,      ///< Request to change power state failed.

    ADSP_IPC_INVALID_MANIFEST = 14,             ///< Manifest of loadable library seems to be invalid

    /* Load/unload library specific codes */
    ADSP_IPC_IMR_TO_SMALL = 42,                 ///< IMR is to small to load library
    ADSP_RSVD_43 = 43,
    ADSP_IPC_CSE_VALIDATION_FAILED = 44,        ///< CSE didn't verified library

    ADSP_IPC_MOD_MGMT_ERROR = 100,              ///< Errors related to module management.
    ADSP_IPC_MOD_LOAD_CL_FAILED = 101,          ///< Loading of a module segment failed.
    ADSP_IPC_MOD_LOAD_INVALID_HASH = 102,       ///< Invalid content of module received, hash does not match.

    ADSP_IPC_MOD_UNLOAD_INST_EXIST = 103,       ///< Request to unload module when its instances are still present.
    ADSP_IPC_MOD_NOT_INITIALIZED = 104,         ///< ModuleInstance was't initialized.
    ADSP_IPC_MOD_STATE_NOT_SET = 105,           ///< ModuleInstance state wasn't set correctly.

    ADSP_IPC_CONFIG_GET_ERROR = 106,            ///< ModuleInstance couldn't get config.
    ADSP_IPC_CONFIG_SET_ERROR = 107,            ///< ModuleInstance couldn't set config.
    ADSP_IPC_LARGE_CONFIG_GET_ERROR = 108,      ///< Failed to retrieve parameter from a module.
    ADSP_IPC_LARGE_CONFIG_SET_ERROR = 109,      ///< Failed to apply parameter to a module.

    ADSP_IPC_MOD_INVALID_ID  = 110,             ///< Passed Module ID is invalid (out of range).
    ADSP_IPC_MOD_INST_INVALID_ID  = 111,        ///< Passed Instance ID is invalid (out of range or not exist).
    ADSP_IPC_QUEUE_INVALID_ID = 112,            ///< Passed src queue ID is invalid (out of range).
    ADSP_IPC_QUEUE_DST_INVALID_ID = 113,        ///< Passed dst queue ID is invalid (out of range).
    ADSP_IPC_BIND_UNBIND_DST_SINK_UNSUPPORTED = 114,///< Bind/Unbind operation is not supported.
    ADSP_IPC_MOD_UNLOAD_INST_EXISTS = 115,      ///< ModuleInstance is using resources that are requested to remove .
    ADSP_IPC_CORE_INVALID_ID = 116,             ///< Passed code ID is invalid.

    ADSP_IPC_INVALID_CONFIG_PARAM_ID = 120,     ///< Invalid configuration parameter ID specified.
    ADSP_IPC_INVALID_CONFIG_DATA_LEN = 121,     ///< Invalid length of configuration parameter specified.
    ADSP_IPC_INVALID_CONFIG_DATA_STRUCT = 122,  ///< Invalid structure of configuration parameter specified.

    ADSP_IPC_GATEWAY_NOT_INITIALIZED = 140,     ///< Gateway initialization error.
    ADSP_IPC_GATEWAY_NOT_EXIST = 141,           ///< Invalid gateway connector ID specified.
    ADSP_IPC_GATEWAY_STATE_NOT_SET = 142,       ///< Failed to change state of the gateway.

    ADSP_IPC_SCLK_ALREADY_RUNNING = 150,        ///< SCLK can be configured when no I2S instance is running.
    ADSP_IPC_MCLK_ALREADY_RUNNING = 151,        ///< MCLK can be configured when no I2S instance is running.
    ADSP_IPC_NO_RUNNING_SCLK = 152,             ///< SCLK can be stopped when at least 1 I2S instance is running.
    ADSP_IPC_NO_RUNNING_MCLK = 153,             ///< MCLK can be stopped when at least 1 I2S instance is running.

    ADSP_IPC_PIPELINE_NOT_INITIALIZED = 160,    ///< Pipeline initialization error.
    ADSP_IPC_PIPELINE_NOT_EXIST = 161,          ///< Invalid pipeline ID specified.
    ADSP_IPC_PIPELINE_SAVE_FAILED = 162,        ///< Pipeline save operation failed.
    ADSP_IPC_PIPELINE_RESTORE_FAILED = 163,     ///< Pipeline restore operation failed.
    ADSP_IPC_PIPELINE_STATE_NOT_SET = 164,      ///< Failed to change state of the pipeline.
    ADSP_IPC_PIPELINE_ALREADY_EXISTS = 165,     ///< Pipeline with passed ID already exists in the system.

    ADSP_IPC_MAX_STATUS = ((1<<IXC_STATUS_BITS)-1)
};

enum class BaseFwParams
{
    /**
     * Use LARGE_CONFIG_GET to retrieve Adsp properties as TLV structure with type
     * of AdspProperties.
     * */
    ADSP_PROPERTIES = 0,
    /**
     * Use LARGE_CONFIG_GET to retrieve resources states (@see PhysMemPages).
     * */
    ADSP_RESOURCE_STATE = 1,
    /**
     * Use LARGE_CONFIG_SET to prepare core(s) for new DX state. Ipc mailbox must
     * contain properly built DxStateInfo struct. Power flows are described in FW HLD.
     * */
    DX_STATE = 2,
    /**
     * Use LARGE_CONFIG_SET to mask/unmask notifications. Ipc mailbox must contain properly
     * built NotificationMaskInfo struct.
     * */
    NOTIFICATION_MASK = 3,
    /**
     * Use LARGE_CONFIG_SET to setup new astate table. Ipc mailbox must contain properly built
     * AstateTable struct.
     * */
    ASTATE_TABLE = 4,
    /**
     * Use LARGE_CONFIG_SET to initialize or modify DMA gateway configuration outside of a
     * stream lifetime.
     * */
    DMA_CONTROL = 5,
    /**
     * Use LARGE_CONFIG_SET to manage logs priorities. Ipc mailbox must contain properly built
     * LogStateInfo structure.
     * */
    ENABLE_LOGS = 6,
    /**
     * Use LARGE_CONFIG_SET/LARGE_CONFIG_GET to write/read FW configuration.
     */
    FW_CONFIG = 7,
    /**
     * Use LARGE_CONFIG_GET to read HW configuration.
     */
    HW_CONFIG_GET = 8,
    /**
     * Use LARGE_CONFIG_GET to read modules configuration.
     */
    MODULES_INFO_GET = 9,
    /**
     * Use LARGE_CONFIG_GET to read pipeline list.
     */
    PIPELINE_LIST_INFO_GET = 10,
    /**
     * Use LARGE_CONFIG_GET to read pipelines properties.
     */
    PIPELINE_PROPS_GET = 11,
    /**
     * Use SCHEDULERS_INFO_GET to read schedulers configuration.
     */
    SCHEDULERS_INFO_GET = 12,
    /**
     * Use LARGE_CONFIG_GET to read gateway configuration.
     */
    GATEWAYS_INFO_GET = 13,
    /**
     * Use LARGE_CONFIG_GET to get information on memory state.
     */
    MEMORY_STATE_INFO_GET = 14,
    /**
     * Use LARGE_CONFIG_GET to get information on power state.
     */
    POWER_STATE_INFO_GET = 15
};

enum class StreamType : uint32_t
{
    STREAM_TYPE_PCM = 0,
    STREAM_TYPE_MP3 = 1
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
    FS_48000HZ = 48000, /**< Default. */
    FS_64000HZ = 64000, /** AAC, SRC only. */
    FS_88200HZ = 88200, /** AAC, SRC only. */
    FS_96000HZ = 96000, /** AAC, SRC only. */
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

enum class InterleavingStyle : uint32_t
{
    CHANNELS_SAMPLES_INTERLEAVING = 0, /*!< [s1_ch1...s1_chN,...,sM_ch1...sM_chN] */
    CHANNELS_BLOCKS_INTERLEAVING = 1, /*!< [s1_ch1...sM_ch1,...,s1_chN...sM_chN] */
};

using ChannelMap = uint32_t;

enum class ChannelConfig : uint32_t
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

enum class SampleType : uint8_t
{
    MSB_INTEGER = 0, /*!< integer with Most Significant Byte first */
    LSB_INTEGER = 1, /*!< integer with Least Significant Byte first */
    SIGNED_INTEGER = 2, /*!< signed integer */
    UNSIGNED_INTEGER = 3, /*!< unsigned integer */
    FLOAT = 4 /*!< unsigned integer */
};

struct AudioDataFormatIpc
{
    SamplingFrequency sampling_frequency; /*!< Sampling frequency in Hz */
    BitDepth bit_depth; /*!< Bit depth of audio samples */
    ChannelMap channel_map; /*!< channel ordering in audio stream */
    ChannelConfig channel_config; /*!< Channel configuration. */
    InterleavingStyle interleaving_style; /*!< The way the samples are interleaved */
    uint8_t number_of_channels; /*!< Total number of channels. */
    uint8_t valid_bit_depth; /*!< Valid bit depth in audio samples */
    SampleType sample_type; /*!< sample type:
                                *  0 - intMSB
                                *  1 - intLSB
                                *  2 - intSinged
                                *  3 - intUnsigned
                                *  4 - float  */
    uint8_t reserved;  /*!< padding byte */

    bool operator == (const AudioDataFormatIpc &other) const
    {
        return bit_depth == other.bit_depth &&
            channel_config == other.channel_config &&
            channel_map == other.channel_map &&
            interleaving_style == other.interleaving_style &&
            number_of_channels == other.number_of_channels &&
            reserved == other.reserved &&
            sample_type == other.sample_type &&
            sampling_frequency == other.sampling_frequency &&
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
};
static_assert(sizeof(AudioDataFormatIpc) == 24, "Wrong AudioDataFormatIpc size");

/* This type exists in the fw header but is not easily includable, so copying it */
enum class BaseModuleParams
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

    bool operator == (const dsp_fw::PinProps &other) const
    {
        return phys_queue_id == other.phys_queue_id &&
            format == other.format &&
            stream_type == other.stream_type;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(stream_type);
        reader.read(format);
        reader.read(phys_queue_id);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(stream_type);
        writer.write(format);
        writer.write(phys_queue_id);
    }

};
static_assert(sizeof(PinProps) == 32, "Wrong PinProps size");

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

    bool operator == (const ConnectorNodeId &other) const
    {
        return val.dw == other.val.dw;
    }
};
static_assert(sizeof(ConnectorNodeId) == 4, "Wrong ConnectorNodeId size");

struct FwVersion
{
    uint16_t major;
    uint16_t minor;
    uint16_t hotfix;
    uint16_t build;

    bool operator == (const FwVersion &other)
    {
        return major == other.major &&
            minor == other.minor &&
            hotfix == other.hotfix &&
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
static_assert(sizeof(FwVersion) == 8, "Wrong FwVersion size");

struct DmaBufferConfig
{
    uint32_t min_size_bytes;
    uint32_t max_size_bytes;

    bool operator == (const DmaBufferConfig &other)
    {
        return min_size_bytes == other.min_size_bytes &&
            max_size_bytes == other.max_size_bytes;
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
static_assert(sizeof(DmaBufferConfig) == 8, "Wrong DmaBufferConfig size");

enum class FwConfigParams
{
    /**
    * Firmware version
    */
    FW_VERSION_FW_CFG = 0,
    /**
    * Indicates whether legacy DMA memory is managed by FW.
    */
    MEMORY_RECLAIMED_FW_CFG = 1,
    /**
    * Frequency of oscillator clock.
    */
    SLOW_CLOCK_FREQ_HZ_FW_CFG = 2,
    /**
    * Frequency of PLL clock.
    */
    FAST_CLOCK_FREQ_HZ_FW_CFG = 3,
    /**
    * List of static and dynamic DMA buffer sizes.
    * SW may configure minimum and maximum size for each buffer.
    */
    DMA_BUFFER_CONFIG_FW_CFG = 4,
    /**
    * Audio Hub Link support level.
    * Note: Lower 16-bits may be used in future to indicate implementation revision if necessary.
    */
    ALH_SUPPORT_LEVEL_FW_CFG = 5,
    /**
    * Size of IPC downlink mailbox.
    */
    IPC_DL_MAILBOX_BYTES_FW_CFG = 6,
    /**
    * Size of IPC uplink mailbox.
    */
    IPC_UL_MAILBOX_BYTES_FW_CFG = 7,
    /**
    * Size of trace log buffer.
    */
    TRACE_LOG_BYTES_FW_CFG = 8,
    /**
    * Maximum number of pipelines that may be instantiated at the same time.
    */
    MAX_PPL_CNT_FW_CFG = 9,
    /**
    * Maximum number of A-state table entries that may be configured by the driver.
    * Driver may also use this value to estimate the size of data retrieved as ASTATE_TABLE property.
    */
    MAX_ASTATE_COUNT_FW_CFG = 10,
    /**
    * Maximum number of input or output pins supported by a module.
    */
    MAX_MODULE_PIN_COUNT_FW_CFG = 11,
    /**
    * Current total number of modules loaded into the DSP.
    */
    MODULES_COUNT_FW_CFG = 12,
    /**
    * Maximum number of tasks supported by a single pipeline.
    */
    MAX_MOD_INST_COUNT_FW_CFG = 13,
    /**
    * Maximum number of LL tasks that may be allocated with
    * the same priority (specified by a priority of the parent pipeline).
    */
    MAX_LL_TASKS_PER_PRI_COUNT_FW_CFG = 14,
    /**
    * Number of LL priorities.
    */
    LL_PRI_COUNT = 15,
    /**
    * Maximum number of DP tasks that may be allocated on a single core.
    */
    MAX_DP_TASKS_COUNT_FW_CFG = 16
};

enum class HwConfigParams
{
    /**
    * Version of cAVS implemented by FW (from ROMInfo).
    */
    cAVS_VER_HW_CFG = 0,
    /**
    * How many dsp cores are available in current audio subsystem.
    */
    DSP_CORES_HW_CFG = 1,
    /**
    * Size of a single memory page.
    */
    MEM_PAGE_BYTES_HW_CFG = 2,
    /**
    * Total number of physical pages available for allocation.
    */
    TOTAL_PHYS_MEM_PAGES_HW_CFG = 3,
    /**
    * SSP capabilities. Number of items in controller_base_addr array is specified by controller_count.
    * Note: Lower 16 bits of I2sVersion may be used in future to indicate implementation revision if necessary.
    */
    I2S_CAPS_HW_CFG = 4,
    /**
    * GPDMA capabilities.
    */
    GPDMA_CAPS_HW_CFG = 5,
    /**
    * Total number of DMA gateways of all types.
    */
    GATEWAY_COUNT_HW_CFG = 6,
    /**
    * Number of SRAM memory banks manageable by DSP.
    */
    EBB_COUNT_HW_CFG = 7
};

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
        uint32_t     auto_start : 1; // 0 - manually created, 1 - single instance created by Module Manager
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

typedef void(*ModuleEntryPoint)(const void*);

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
    uint16_t    instance_stack_size;        // size of stack that instance requires for its task (DP) [bytes]
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
    using ModuleCountType = uint32_t;

    std::vector<ModuleEntry> module_info;

    static std::size_t getAllocationSize(std::size_t count) {
        return sizeof(ModuleCountType) + count * sizeof(ModuleEntry);
    }

    bool operator == (const ModulesInfo &other)
    {
        return module_info == other.module_info ;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVectorAndRecurse<ModuleCountType>(module_info);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVectorAndRecurse<ModuleCountType>(module_info);
    }
};

struct GatewayProps
{
    /**
    * Gateway ID (refer to ConnectorNodeId).
    */
    uint32_t  id;
    /**
    * Gateway attributes (refer to GatewayAttributes).
    */
    uint32_t  attribs;

    bool operator == (const GatewayProps &other) const
    {
        return id == other.id &&
            attribs == other.attribs;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(id);
        reader.read(attribs);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(id);
        writer.write(attribs);
    }
};
static_assert(sizeof(GatewayProps) == 8, "Wrong GatewayProps size");

struct GatewaysInfo
{
    using ModuleCountType = uint32_t;

    std::vector<GatewayProps> gateways;

    static std::size_t getAllocationSize(std::size_t count) {
        return sizeof(ModuleCountType) + count * sizeof(GatewayProps);
    }

    bool operator == (const GatewaysInfo &other) const
    {
        return gateways == other.gateways;
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.readVectorAndRecurse<ModuleCountType>(gateways);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.writeVectorAndRecurse<ModuleCountType>(gateways);
    }
};

}
}
}
