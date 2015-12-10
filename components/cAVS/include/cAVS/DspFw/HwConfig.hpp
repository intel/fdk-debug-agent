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
    /**
     * @fixme this type definition should be removed and replaced by the one from FW in
     * basefw_config.h.
     */
    struct GpdmaCapabilities
    {
        uint32_t lp_gpdma0_count;
        uint32_t lp_gpdma1_count;
        uint32_t hp_gpdma_count;
    };

    struct I2sCaps final
    {
        uint32_t version;
        std::vector<uint32_t> controllerBaseAddr;
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
            std::make_unique<I2sCapsTlvWrapper>(i2sCaps, isI2sCapsValid);
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

    /**
     * A dedicated TLV wrapper for type I2sCapabilities.
     * @see I2sCapabilities
     * @remarks this inner class should be private, but have it public allows to unit test it.
     */
    class I2sCapsTlvWrapper final : public tlv::TlvWrapperInterface
    {
    public:
        /**
         * @param[in] value the reference to the shadow runtime variable I2sCaps
         * @param[in] valid the reference to the shadow runtime variable valid flag
         */
        I2sCapsTlvWrapper(I2sCaps &value, bool &valid) : mValue(value), mValid(valid)
        {
            invalidate();
        }

        /**
         * A size would be consider as valid if the size is equal to
         * minimumI2sCapsSize + n x sizeof(uint32_t)
         */
        virtual bool isValidSize(size_t binaryValueSize) const NOEXCEPT override
        {
            if (binaryValueSize >= minimumI2sCapsSize) {

                binaryValueSize -= minimumI2sCapsSize;
                return (binaryValueSize % sizeof(uint32_t)) == 0;
            }
            return false;
        }

        virtual void readFrom(const char *binarySource, size_t binaryValueSize) override
        {
            if (!isValidSize(binaryValueSize)) {

                throw Exception(std::string("Invalid binary size (") +
                                std::to_string(binaryValueSize) +
                                " bytes) for a TLV I2sCaps value");
            }
            // First uint32_t is version
            mValue.version = *(reinterpret_cast<const uint32_t *>(binarySource));
            binarySource += sizeof(uint32_t);
            // Second uint32_t is controller_count
            uint32_t controllerCount = *(reinterpret_cast<const uint32_t *>(binarySource));
            if (!isValidSize(binaryValueSize, controllerCount)) {

                throw tlv::TlvWrapperInterface::Exception("struct I2sCapabilities inconsistency");
            }
            binarySource += sizeof(uint32_t);
            // Then as much uint32_t as controller_count values
            mValue.controllerBaseAddr.resize(controllerCount);
            for (uint32_t i = 0; i < controllerCount; i++) {

                uint32_t controllerBaseAddr = *(reinterpret_cast<const uint32_t *>(binarySource));
                binarySource += sizeof(uint32_t);
                mValue.controllerBaseAddr[i] = controllerBaseAddr;
            }

            mValid = true;
        }

        virtual void invalidate() NOEXCEPT override
        {
            mValue.controllerBaseAddr.clear();
            mValid = false;
        }

    private:
        /**
         * A much better size checker which can be used by I2sCapsTlvWrapper::readFrom since it can
         * check value consistency: the controller_base_addr array number of element must be equal
         * to controllerCount.
         * @param[in] binaryValueSize a binary size to be validated
         * @param[in] controllerCount the I2sCapabilities.controller_count value
         * @see I2sCapsTlvWrapper::readFrom
         */
        bool isValidSize(size_t binaryValueSize, uint32_t controllerCount) const NOEXCEPT
        {
            if (isValidSize(binaryValueSize)) {

                // Check that array size is correct regarding controllerCount
                binaryValueSize -= minimumI2sCapsSize;
                return binaryValueSize == (controllerCount * sizeof(uint32_t));
            }
            return false;
        }

        /*
         * A I2sCapabilities value minimum size is 8 bytes
         * (version + controller_count fields)
         */
        static const size_t minimumI2sCapsSize = 8;

        I2sCaps &mValue;
        bool &mValid;
    };

private:
    tlv::TlvDictionary<HwConfigParams>::TlvMap mHwConfigTlvMap;
    tlv::TlvDictionary<HwConfigParams> mHwConfigTlvDictionary;
};
}
}
}