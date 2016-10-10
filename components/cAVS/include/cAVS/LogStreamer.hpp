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

#include <cAVS/Logger.hpp>
#include "cAVS/Driver.hpp"
#include <System/IfdkStreamer.hpp>
#include <ostream>
#include <string>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{

class LogStreamer final : public system::IfdkStreamer
{
public:
    /**
     * A LogStreamer writes the cAVS log to an ostream in real time, using a cavs::Logger interface.
     * @param[in] logger The Logger interface to be used to get the cAVS log
     * @param[in] moduleEntries The FW module entries table
     * @throw Streamer::Exception
     * @todo The LogStreamer will need a way to retrieve the "Module Entries" table in a subsequent
     * patch.
     */
    LogStreamer(Logger &logger, const std::vector<dsp_fw::ModuleEntry> &moduleEntries);

private:
    virtual void streamFormatHeader(std::ostream &os) override;
    virtual bool streamNextFormatData(std::ostream &os) override;

    /**
     * The Logger interface to be used to get the cAVS log
     */
    Logger &mLogger;

    /**
     * The module entries table retrieved from FW once, at initialization
     */
    const std::vector<dsp_fw::ModuleEntry> &mModuleEntries;

    /**
     * The stream is produced in the IFDK stream format which requires a system type (cavs)
     * @todo this system type should be used in multiple places in cAVS in the near future,
     * consider to define it somewhere else reachable from cavs::LogStreamer.
     */
    static const std::string systemType;

    /**
     * The stream is produced in the IFDK stream format which requires a format type (fwlog)
     */
    static const std::string formatType;

    /**
     * The IFDK:cavs:fwlog format major version
     */
    static const int majorVersion;

    /**
     * The IFDK:cavs:fwlog format minor version
     */
    static const int minorVersion;

    /* Make this class non copyable */
    LogStreamer(const LogStreamer &) = delete;
    LogStreamer &operator=(const LogStreamer &) = delete;
};
}
}