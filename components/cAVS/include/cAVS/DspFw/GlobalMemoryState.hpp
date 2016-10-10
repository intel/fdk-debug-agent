/*
 * Copyright (c) 2016, Intel Corporation
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
