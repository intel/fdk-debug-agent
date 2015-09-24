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

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

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

}
}
}
