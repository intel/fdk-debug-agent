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

#include "cAVS/DspFw/ConfigTypes.hpp"
#include "Tlv/TlvResponseHandlerInterface.hpp"
#include "Tlv/TlvDictionary.hpp"
#include "Tlv/TlvWrapper.hpp"
#include "Tlv/TlvVectorWrapper.hpp"

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/**
 * The class FwConfig defines all parameters that can be provided as TLV response for the
 * Firmware Config request to the cAVS FW.
 * For each FW Config parameter, a boolean valid flag indicates if the value is valid or not.
 * A Tlv::TlvUnpack instance can be used to updates a FwConfig instance from a binary TLV list
 * returned by the cAVS FW thanks to the Tlv::TlvDictionaryInterface exposed by the FwConfig
 * instance.
 */
class FwConfig final : public tlv::TlvResponseHandlerInterface
{
public:
    FwVersion fwVersion;
    bool isFwVersionValid;

    uint32_t memoryReclaimed;
    bool isMemoryReclaimedValid;

    uint32_t slowClockFreqHz;
    bool isSlowClockFreqHzValid;

    uint32_t fastClockFreqHz;
    bool isFastClockFreqHzValid;

    std::vector<DmaBufferConfig> dmaBufferConfig;

    /* We do not use the FW type 'enum AlhSupportLevel' since we cannot guarantee that the
     * debug agent will consume 4 bytes for an enum. */
    uint32_t alhSupportLevel;
    bool isAlhSupportLevelValid;

    uint32_t ipcDlMailboxBytes;
    bool isIpcDlMailboxBytesValid;

    uint32_t ipcUlMailboxBytes;
    bool isIpcUlMailboxBytesValid;

    uint32_t traceLogBytes;
    bool isTraceLogBytesValid;

    uint32_t maxPplCount;
    bool isMaxPplCountValid;

    uint32_t maxAstateCount;
    bool isMaxAstateCountValid;

    uint32_t maxModulePinCount;
    bool isMaxModulePinCountValid;

    uint32_t modulesCount;
    bool isModulesCountValid;

    uint32_t maxModInstCount;
    bool isMaxModInstCountValid;

    uint32_t maxLlTasksPerPriCount;
    bool isMaxLlTasksPerPriCountValid;

    uint32_t llPriCount;
    bool isLlPriCountValid;

    uint32_t maxDpTasksCount;
    bool isMaxDpTasksCountValid;

    FwConfig() : mFwConfigTlvMap(), mFwConfigTlvDictionary(mFwConfigTlvMap)
    {
        using Tags = FwConfigParams;
        using namespace tlv;

        mFwConfigTlvMap[Tags::FW_VERSION_FW_CFG] =
            std::make_unique<TlvWrapper<FwVersion>>(fwVersion, isFwVersionValid);
        mFwConfigTlvMap[Tags::MEMORY_RECLAIMED_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(memoryReclaimed, isMemoryReclaimedValid);
        mFwConfigTlvMap[Tags::SLOW_CLOCK_FREQ_HZ_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(slowClockFreqHz, isSlowClockFreqHzValid);
        mFwConfigTlvMap[Tags::FAST_CLOCK_FREQ_HZ_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(fastClockFreqHz, isFastClockFreqHzValid);
        mFwConfigTlvMap[Tags::DMA_BUFFER_CONFIG_FW_CFG] =
            std::make_unique<TlvVectorWrapper<DmaBufferConfig>>(dmaBufferConfig);
        mFwConfigTlvMap[Tags::ALH_SUPPORT_LEVEL_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(alhSupportLevel, isAlhSupportLevelValid);
        mFwConfigTlvMap[Tags::IPC_DL_MAILBOX_BYTES_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(ipcDlMailboxBytes, isIpcDlMailboxBytesValid);
        mFwConfigTlvMap[Tags::IPC_UL_MAILBOX_BYTES_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(ipcUlMailboxBytes, isIpcUlMailboxBytesValid);
        mFwConfigTlvMap[Tags::TRACE_LOG_BYTES_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(traceLogBytes, isTraceLogBytesValid);
        mFwConfigTlvMap[Tags::MAX_PPL_CNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxPplCount, isMaxPplCountValid);
        mFwConfigTlvMap[Tags::MAX_ASTATE_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxAstateCount, isMaxAstateCountValid);
        mFwConfigTlvMap[Tags::MAX_MODULE_PIN_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxModulePinCount, isMaxModulePinCountValid);
        mFwConfigTlvMap[Tags::MODULES_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(modulesCount, isModulesCountValid);
        mFwConfigTlvMap[Tags::MAX_MOD_INST_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxModInstCount, isMaxModInstCountValid);
        mFwConfigTlvMap[Tags::MAX_LL_TASKS_PER_PRI_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxLlTasksPerPriCount,
                                                   isMaxLlTasksPerPriCountValid);
        mFwConfigTlvMap[Tags::LL_PRI_COUNT] =
            std::make_unique<TlvWrapper<uint32_t>>(llPriCount, isLlPriCountValid);
        mFwConfigTlvMap[Tags::MAX_DP_TASKS_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxDpTasksCount, isMaxDpTasksCountValid);
    }

    const tlv::TlvDictionaryInterface &getTlvDictionary() const noexcept override
    {
        return mFwConfigTlvDictionary;
    }

private:
    tlv::TlvDictionary<FwConfigParams>::TlvMap mFwConfigTlvMap;
    tlv::TlvDictionary<FwConfigParams> mFwConfigTlvDictionary;
};
}
}
}