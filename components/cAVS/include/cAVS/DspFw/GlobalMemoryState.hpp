/*
 ********************************************************************************
 *                              INTEL CONFIDENTIAL
 *   Copyright(C) 2016 Intel Corporation. All Rights Reserved.
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
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/StructureChangeTracking.hpp"
#include "Tlv/TlvResponseHandlerInterface.hpp"
#include "Tlv/TlvDictionary.hpp"
#include "Tlv/TlvWrapper.hpp"
#include <vector>
#include <utility>

namespace debug_agent
{
namespace cavs
{
namespace dsp_fw
{
// The sum of the size of its members is 18 but since it isn't packed, it is padded to 20
CHECK_SIZE(private_fw::SramStateInfo, 20);
CHECK_MEMBER(private_fw::SramStateInfo, free_phys_mem_pages, 0, uint32_t);
CHECK_MEMBER(private_fw::SramStateInfo, ebb_state_dword_count, 4, uint32_t);
CHECK_MEMBER(private_fw::SramStateInfo, ebb_state, 8, uint32_t[1]);
CHECK_MEMBER(private_fw::SramStateInfo, page_alloc_count, 12, uint32_t);
CHECK_MEMBER(private_fw::SramStateInfo, page_alloc, 16, uint16_t[1]);
class GlobalMemoryState : public tlv::TlvResponseHandlerInterface
{
public:
    struct SramStateInfo
    {
        uint32_t freePhysMemPages;
        // Caveat: each entry of this vector is actually a bitset and the actual state is the
        // concatenation of all these bitsets.
        std::vector<uint32_t> ebbStates;
        // Caveat: each entry of this vector is actually a bitset and the actual state is the
        // concatenation of all these bitsets.
        std::vector<uint16_t> pageAlloc;

        bool operator==(const SramStateInfo &other) const
        {
            return freePhysMemPages == other.freePhysMemPages && ebbStates == other.ebbStates &&
                   pageAlloc == other.pageAlloc;
        }

        void fromStream(util::ByteStreamReader &reader)
        {
            reader.read(freePhysMemPages);
            reader.readVector<ArraySizeType>(ebbStates);
            reader.readVector<ArraySizeType>(pageAlloc);
        }

        void toStream(util::ByteStreamWriter &writer)
        {
            writer.write(freePhysMemPages);
            writer.writeVector<ArraySizeType>(ebbStates);
            writer.writeVector<ArraySizeType>(pageAlloc);
        }
    };

    /** This enum is the T in "TLV" */
    enum class MemoryState : uint32_t
    {
        Lpsram = private_fw::LPSRAM_STATE,
        Hpsram = private_fw::HPSRAM_STATE
    };

    SramStateInfo lpsramState;
    bool isLpsramStateValid;

    SramStateInfo hpsramState;
    bool isHpsramStateValid;

    GlobalMemoryState() : mTlvDictionary(mkMap()) {}

    const tlv::TlvDictionaryInterface &getTlvDictionary() const noexcept override
    {
        return mTlvDictionary;
    }

private:
    using Dictionary = tlv::TlvDictionary<MemoryState>;

    Dictionary::TlvMap mkMap()
    {
        using namespace tlv;

        Dictionary::TlvMap tlvMap;
        tlvMap[MemoryState::Lpsram] =
            std::make_unique<TlvWrapper<SramStateInfo>>(lpsramState, isLpsramStateValid);
        tlvMap[MemoryState::Hpsram] =
            std::make_unique<TlvWrapper<SramStateInfo>>(hpsramState, isHpsramStateValid);

        return tlvMap;
    }

    Dictionary mTlvDictionary;
};
} // dsp_fw
} // cavs
} // debug_agent
