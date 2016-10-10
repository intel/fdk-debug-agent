/*
 * Copyright (c) 2015-2016, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

    uint32_t maxLibsCount;
    bool isMaxLibsCountValid;

    FwConfig() : mFwConfigTlvDictionary(mkMap()) {}

    const tlv::TlvDictionaryInterface &getTlvDictionary() const noexcept override
    {
        return mFwConfigTlvDictionary;
    }

private:
    using Dictionary = tlv::TlvDictionary<FwConfigParams>;

    Dictionary::TlvMap mkMap()
    {
        using Tags = FwConfigParams;
        using namespace tlv;

        Dictionary::TlvMap tlvMap;
        tlvMap[Tags::FW_VERSION_FW_CFG] =
            std::make_unique<TlvWrapper<FwVersion>>(fwVersion, isFwVersionValid);
        tlvMap[Tags::MEMORY_RECLAIMED_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(memoryReclaimed, isMemoryReclaimedValid);
        tlvMap[Tags::SLOW_CLOCK_FREQ_HZ_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(slowClockFreqHz, isSlowClockFreqHzValid);
        tlvMap[Tags::FAST_CLOCK_FREQ_HZ_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(fastClockFreqHz, isFastClockFreqHzValid);
        tlvMap[Tags::DMA_BUFFER_CONFIG_FW_CFG] =
            std::make_unique<TlvVectorWrapper<DmaBufferConfig>>(dmaBufferConfig);
        tlvMap[Tags::ALH_SUPPORT_LEVEL_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(alhSupportLevel, isAlhSupportLevelValid);
        tlvMap[Tags::IPC_DL_MAILBOX_BYTES_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(ipcDlMailboxBytes, isIpcDlMailboxBytesValid);
        tlvMap[Tags::IPC_UL_MAILBOX_BYTES_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(ipcUlMailboxBytes, isIpcUlMailboxBytesValid);
        tlvMap[Tags::TRACE_LOG_BYTES_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(traceLogBytes, isTraceLogBytesValid);
        tlvMap[Tags::MAX_PPL_CNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxPplCount, isMaxPplCountValid);
        tlvMap[Tags::MAX_ASTATE_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxAstateCount, isMaxAstateCountValid);
        tlvMap[Tags::MAX_MODULE_PIN_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxModulePinCount, isMaxModulePinCountValid);
        tlvMap[Tags::MODULES_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(modulesCount, isModulesCountValid);
        tlvMap[Tags::MAX_MOD_INST_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxModInstCount, isMaxModInstCountValid);
        tlvMap[Tags::MAX_LL_TASKS_PER_PRI_COUNT_FW_CFG] = std::make_unique<TlvWrapper<uint32_t>>(
            maxLlTasksPerPriCount, isMaxLlTasksPerPriCountValid);
        tlvMap[Tags::LL_PRI_COUNT] =
            std::make_unique<TlvWrapper<uint32_t>>(llPriCount, isLlPriCountValid);
        tlvMap[Tags::MAX_DP_TASKS_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxDpTasksCount, isMaxDpTasksCountValid);
        tlvMap[Tags::MAX_LIBS_COUNT_FW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(maxLibsCount, isMaxLibsCountValid);

        return tlvMap;
    }
    Dictionary mFwConfigTlvDictionary;
};
}
}
}
