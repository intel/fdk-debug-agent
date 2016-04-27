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
#include "Util/StructureChangeTracking.hpp"
#include <vector>
#include <string>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{

/**
 * Defines all parameters that can be provided as TLV response for the
 * Hardware Config request to the cAVS FW.
 *
 * For each HW Config parameter, a boolean valid flag indicates if the value is valid or not.
 * A Tlv::TlvUnpack instance can be used to updates a HwConfig instance from a binary TLV list
 * returned by the cAVS FW thanks to the Tlv::TlvDictionaryInterface exposed by the HwConfig
 * instance.
 *
 * @todo this version ignores tag GPDMA_CAPS (5) since definition and FW types are not consistent
 */
class HwConfig final : public tlv::TlvResponseHandlerInterface
{
public:
    /* GpdmaCapabilities */

    CHECK_SIZE(private_fw::GpdmaCapabilities, 16);
    CHECK_MEMBER(private_fw::GpdmaCapabilities, lp_ctrl_count, 0, uint32_t);
    CHECK_MEMBER(private_fw::GpdmaCapabilities, lp_ch_count, 4, uint32_t[1]);
    CHECK_MEMBER(private_fw::GpdmaCapabilities, hp_ctrl_count, 8, uint32_t);
    CHECK_MEMBER(private_fw::GpdmaCapabilities, hp_ch_count, 12, uint32_t[1]);

    struct GpdmaCapabilities
    {
        std::vector<uint32_t> lp_gateways;
        std::vector<uint32_t> hp_gateways;

        bool operator==(const GpdmaCapabilities &other) const
        {
            return lp_gateways == other.lp_gateways && hp_gateways == other.hp_gateways;
        }

        void fromStream(util::ByteStreamReader &reader)
        {
            reader.readVector<ArraySizeType>(lp_gateways);
            reader.readVector<ArraySizeType>(hp_gateways);
        }
    };

    /* I2sCapabilities */

    CHECK_SIZE(private_fw::I2sCapabilities, 12);
    CHECK_MEMBER(private_fw::I2sCapabilities, version, 0, private_fw::I2sVersion);
    CHECK_MEMBER(private_fw::I2sCapabilities, controller_count, 4, uint32_t);
    CHECK_MEMBER(private_fw::I2sCapabilities, controller_base_addr, 8, uint32_t[1]);

    struct I2sCapabilities final
    {
        uint32_t version;
        std::vector<uint32_t> controllerBaseAddr;

        bool operator==(const I2sCapabilities &other) const
        {
            return version == other.version && controllerBaseAddr == other.controllerBaseAddr;
        }

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

    I2sCapabilities i2sCaps;
    bool isI2sCapsValid;

    GpdmaCapabilities gpdmaCaps;
    bool isGpdmaCapsValid;

    uint32_t gatewayCount;
    bool isGatewayCountValid;

    uint32_t hpEbbCount;
    bool isHpEbbCountValid;

    uint32_t lpEbbCount;
    bool isLpEbbCountValid;

    uint32_t ebbSizeBytes;
    bool isEbbSizeBytesValid;

    HwConfig() : mHwConfigTlvDictionary(mkMap()) {}

    const tlv::TlvDictionaryInterface &getTlvDictionary() const noexcept override
    {
        return mHwConfigTlvDictionary;
    }

private:
    using Dictionary = tlv::TlvDictionary<HwConfigParams>;

    Dictionary::TlvMap mkMap()
    {
        using Tags = HwConfigParams;
        using namespace tlv;

        Dictionary::TlvMap tlvMap;
        tlvMap[Tags::cAVS_VER_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(cavsVersion, isCavsVersionValid);
        tlvMap[Tags::DSP_CORES_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(dspCoreCount, isDspCoreCountValid);
        tlvMap[Tags::MEM_PAGE_BYTES_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(memPageSize, isMemPageSizeValid);
        tlvMap[Tags::TOTAL_PHYS_MEM_PAGES_HW_CFG] = std::make_unique<TlvWrapper<uint32_t>>(
            totalPhysicalMemoryPage, isTotalPhysicalMemoryPageValid);
        tlvMap[Tags::I2S_CAPS_HW_CFG] =
            std::make_unique<TlvWrapper<I2sCapabilities>>(i2sCaps, isI2sCapsValid);
        tlvMap[Tags::GPDMA_CAPS_HW_CFG] =
            std::make_unique<TlvWrapper<GpdmaCapabilities>>(gpdmaCaps, isGpdmaCapsValid);
        tlvMap[Tags::GATEWAY_COUNT_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(gatewayCount, isGatewayCountValid);
        tlvMap[Tags::LP_EBB_COUNT_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(lpEbbCount, isLpEbbCountValid);
        tlvMap[Tags::HP_EBB_COUNT_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(hpEbbCount, isHpEbbCountValid);
        tlvMap[Tags::EBB_SIZE_BYTES_HW_CFG] =
            std::make_unique<TlvWrapper<uint32_t>>(ebbSizeBytes, isEbbSizeBytesValid);

        return tlvMap;
    }

    Dictionary mHwConfigTlvDictionary;
};
}
}
}
