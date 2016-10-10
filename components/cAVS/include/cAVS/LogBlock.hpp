/*
 * Copyright (c) 2015, Intel Corporation
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

#include "Util/Buffer.hpp"
#include <vector>
#include <iostream>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{

/**
 * Implement the cAVS log block as defined by the SwAS.
 *
 * @tparam maxSize maximum size of log data in block
 */
template <const size_t maxSize>
class LogBlockBase final
{
public:
    struct Exception final : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /**
     * Log data are stored into a buffer
     */
    using LogData = util::Buffer;

    /**
     * @param[in] coreId The ID of the log block producer core (in [0..15])
     * @param[in] length The buffer length to be preallocated for log data (0 if not specified)
     * @throw LogBlockBase::Exception
     */
    LogBlockBase(unsigned int coreId, std::size_t length = 0) : mCoreId(coreId), mLogData(length)
    {
        if (mCoreId > maxCoreId) {

            throw Exception("Invalid Core ID: " + std::to_string(mCoreId) + " should be in [0.." +
                            std::to_string(maxCoreId) + ']');
        }
    }

    /**
     * @return log block data as const
     */
    const LogData &getLogData() const noexcept { return mLogData; }

    /**
     * @return log block data
     */
    LogData &getLogData() noexcept { return mLogData; }

    /**
     * @return log block Core ID
     */
    unsigned int getCoreId() const noexcept { return mCoreId; }

    /**
     * @return log block size in bytes
     */
    std::size_t getLogSize() const noexcept { return mLogData.size(); }

    /**
     * Serialize the log block to an ostream
     *
     * @tparam _maxSize maximum size of log data in block
     * @param[in] os the ostream
     * @param[in] logBlock the log block to be serialized
     * @return ostream containing original ostream plus serialized log block
     * @throw LogBlockBase::Exception
     */
    template <const size_t _maxSize>
    friend std::ostream &operator<<(std::ostream &os, const LogBlockBase<_maxSize> &logBlock);
    /* The template parameter of this friend function cannot be called 'maxSize' since it would
     * shadow the class template parameter 'maxSize'. For that reason, it must be called with
     * another name. Here it is prefixed with '_' because it would be strange to name it differently
     * while it is for the same purpose!
     */

private:
    /**
     * Each log block is produced by a core referenced by its ID.
     * mCoreId stores the ID of the core which has produced this log block.
     */
    unsigned int mCoreId;

    /**
     * Maximum core ID value
     */
    static const unsigned int maxCoreId = 15;

    /**
     * Log data contained by this log block
     */
    LogData mLogData;

    /* Make this class non copyable */
    LogBlockBase(const LogBlockBase &) = delete;
    LogBlockBase &operator=(const LogBlockBase &) = delete;
};

/**
 * Define the cavs::LogBlock using maximum size defined by SwAS.
 */
static const size_t cavsLogBlockMaxSize = 0x0000FFFF;
using LogBlock = LogBlockBase<cavsLogBlockMaxSize>;

template <const size_t maxSize>
std::ostream &operator<<(std::ostream &os, const LogBlockBase<maxSize> &logBlock)
{
    static const unsigned int coreIdBitOffset = 28;

    if (logBlock.getLogSize() > maxSize) {

        throw typename LogBlockBase<maxSize>::Exception("Log block size exceeds maximum value");
    }
    /* Prepare block header: CoreID and block Length on same 32bits word */
    uint32_t blockHeader = static_cast<uint32_t>(logBlock.getCoreId()) << coreIdBitOffset;
    blockHeader |= static_cast<uint32_t>(logBlock.getLogSize());

    /* Serialize block header */
    os.write(reinterpret_cast<const char *>(&blockHeader), sizeof(blockHeader));

    /* Serialize block log data */
    os.write(reinterpret_cast<const char *>(logBlock.getLogData().data()), logBlock.getLogSize());

    return os;
}
}
}
