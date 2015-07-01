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

#include <vector>
#include <iostream>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{

/**
 * Implement the cAVS log block as defined by the SwAS.
 * @tparam[in] maxSize maximum size of log data in block
 */
template <const size_t maxSize>
class LogBlockBase final
{
public:
    class Exception final : public std::logic_error
    {
    public:
        explicit Exception(const std::string &what)
        : std::logic_error(what)
        {}
    };

    /**
     * Log data are stored into a std::vector<char>
     */
     using LogData = std::vector<char>;

    /**
     * @param[in] coreId The ID of the log block producer core (in [0..15])
     * @param[in] length The buffer length to be preallocated for log data (0 if not specified)
     * @throw LogBlockBase::Exception
     */
    LogBlockBase(unsigned int coreId, std::size_t length = 0) :
        mCoreId(coreId),
        mLogData(length)
    {
        if (mCoreId > maxCoreId) {

            throw Exception("Invalid Core ID: "
                + std::to_string(mCoreId)
                + " should be in [0.."
                + std::to_string(maxCoreId) + ']');
        }
    }

    /**
     * @return log block data as const
     */
    const LogData &getLogData() const NOEXCEPT
    {
        return mLogData;
    }

    /**
     * @return log block data
     */
    LogData &getLogData() NOEXCEPT
    {
        return mLogData;
    }

    /**
     * @return log block Core ID
     */
    unsigned int getCoreId() const NOEXCEPT
    {
        return mCoreId;
    }

    /**
     * @return log block size in bytes
     */
    std::size_t getLogSize() const NOEXCEPT
    {
        return mLogData.size();
    }

    /**
     * Serialize the log block to an ostream
     * @tparam[in] _maxSize maximum size of log data in block
     * @param[in] os the ostream
     * @param[in] logBlock the log block to be serialized
     * @return ostream containing original ostream plus serialized log block
     * @throw LogBlockBase::Exception
     */
    template <const size_t _maxSize> friend
    std::ostream &operator<<(std::ostream &os, const LogBlockBase<_maxSize> &logBlock);
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
    LogBlockBase & operator=(const LogBlockBase &) = delete;
};

/**
 * Define the cavs::LogBlock using maximum size defined by SwAS.
 */
static const size_t cavsLogBlockMaxSize = 0x0FFFFFFF;
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
    os.write(reinterpret_cast<char *>(&blockHeader), sizeof(blockHeader));

    /* Serialize block log data */
    os.write(logBlock.getLogData().data(), logBlock.getLogSize());

    return os;
}

}
}
