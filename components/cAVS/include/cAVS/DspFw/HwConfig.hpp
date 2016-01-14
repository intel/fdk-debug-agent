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

#include "cAVS/DspFw/Common.hpp"
#include "cAVS/DspFw/ConfigTypes.hpp"
#include "Tlv/TlvResponseHandlerInterface.hpp"
#include "Tlv/TlvDictionary.hpp"
#include "Tlv/TlvWrapper.hpp"
#include <vector>
#include <string>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/**
 * The class HwConfig defines all parameters that can be provided as TLV response for the
 * Hardware Config request to the cAVS FW.
 * For each HW Config parameter, a boolean valid flag indicates if the value is valid or not.
 * A Tlv::TlvUnpack instance can be used to updates a HwConfig instance from a binary TLV list
 * returned by the cAVS FW thanks to the Tlv::TlvDictionaryInterface exposed by the HwConfig
 * instance.
 * @fixme this version ignores tag GPDMA_CAPS (5) since definition and FW types are not consistent
 */
class HwConfig final : public tlv::TlvResponseHandlerInterface
{
public:
    struct GpdmaCapabilities
    {
        uint32_t lp_gpdma0_count;
        uint32_t lp_gpdma1_count;
        uint32_t hp_gpdma_count;

        void fromStream(util::ByteStreamReader &reader)
        {
            reader.read(lp_gpdma0_count);
            reader.read(lp_gpdma1_count);
            reader.read(hp_gpdma_count);
        }
    };

    struct I2sCaps final
    {
        uint32_t version;
        std::vector<uint32_t> controllerBaseAddr;

        void fromStream(util::ByteStreamReader &reader)
        {
            reader.read(version);
            reader.readVector<ArraySizeType>(controllerBaseAddr);
        }
    };

    /* We do not use the FW type 'enum CavsVersion' since we cannot guarantee that the
     * debug agent will consume 4 bytes for an enum. */
    uint32_t cavsVersion;
    bool isCavsVersionValid;

    uint32_t dspCoreCount;
    bool isDspCoreCountValid;

    uint32_t memPageSize;
    bool isMemPageSizeValid;

    uint32_t totalPhysicalMemoryPage;
    bool isTotalPhysicalMemoryPageValid;

    I2sCaps i2sCaps;
    bool isI2sCapsValid;

    GpdmaCapabilities gpdmaCaps;
    bool isGpdmaCapsValid;

    uint32_t gatewayCount;
    bool isGatewayCountValid;

    uint32_t ebbCount;
    bool isEbbCountValid;

    HwConfig() : mHwConfigTlvMap(), mHwConfigTlvDictionary(mHwConfigTlvMap)
    {
        using Tags = HwConfigParams;
        using namespace tlv;

        mHwConfigTlvMap[Tags::cAVS_VER_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(cavsVersion, isCavsVersionValid);
        mHwConfigTlvMap[Tags::DSP_CORES_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(dspCoreCount, isDspCoreCountValid);
        mHwConfigTlvMap[Tags::MEM_PAGE_BYTES_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(memPageSize, isMemPageSizeValid);
        mHwConfigTlvMap[Tags::TOTAL_PHYS_MEM_PAGES_HW_CFG] = std::make_unique<TlvWrapper<uint32_t>>(
            totalPhysicalMemoryPage, isTotalPhysicalMemoryPageValid);
        mHwConfigTlvMap[Tags::I2S_CAPS_HW_CFG] =
            std::make_unique<TlvWrapper<I2sCaps>>(i2sCaps, isI2sCapsValid);
        mHwConfigTlvMap[Tags::GPDMA_CAPS_HW_CFG] =
            std::make_unique<TlvWrapper<GpdmaCapabilities>>(gpdmaCaps, isGpdmaCapsValid);
        mHwConfigTlvMap[Tags::GATEWAY_COUNT_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(gatewayCount, isGatewayCountValid);
        mHwConfigTlvMap[Tags::EBB_COUNT_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(ebbCount, isEbbCountValid);
    }

    const tlv::TlvDictionaryInterface &getTlvDictionary() const NOEXCEPT override
    {
        return mHwConfigTlvDictionary;
    }

private:
    tlv::TlvDictionary<HwConfigParams>::TlvMap mHwConfigTlvMap;
    tlv::TlvDictionary<HwConfigParams> mHwConfigTlvDictionary;
};
}
}
}