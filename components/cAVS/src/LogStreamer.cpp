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
#include <cAVS/LogStreamer.hpp>
#include <memory>
#include <cstdint>

namespace debug_agent
{
namespace cavs
{

/**
 * Serialize a partial module entries table to an ostream. The table is partial since only fields
 * required for IFDK:cavs:fwlog format will be serialized.
 * @param[in] os the ostream
 * @param[in] moduleEntries the module entries table to be serialized
 * @return ostream containing original ostream plus serialized IfdkStreamHeader
 */
std::ostream &operator<<(std::ostream &os, const std::vector<dsp_fw::ModuleEntry> &moduleEntries)
{
    /**
     * @fixme For each entry, versions (major, minor, build and hotfix) have to be streamed out.
     * The issue is that these versions are not yet available: zeros are streamed instead.
     *
     * @warning this code is designed for little endian machine like x86 and will not produce
     * correct format on big endian machine.
     */
    static const uint16_t versionMinor = 0;
    static const uint16_t versionMajor = 0;
    static const uint16_t versionBuild = 0;
    static const uint16_t versionHotFix = 0;

    for (auto moduleEntry : moduleEntries) {
        /* The Module ID must be streamed out as 32bits word little endian. */
        uint32_t moduleId = moduleEntry.module_id;
        os.write(reinterpret_cast<char *>(&moduleId), sizeof(moduleId));

        /* The module UUID must be streamed out in big endian, but is is already formatted in
         * big endian in the structure coming from the FW: we can stream out directly.
         */
        static const size_t uuidWords = 4;
        for (size_t i = 0; i < uuidWords; ++i) {

            os.write(reinterpret_cast<char *>(&moduleEntry.uuid[i]), sizeof(moduleEntry.uuid[i]));
        }

        /* Versions must be streamed out in little endian: streaming them from IA memory in which
         * they are in little endian.
         */
        os.write(reinterpret_cast<const char *>(&versionMinor), sizeof(versionMinor));
        os.write(reinterpret_cast<const char *>(&versionMajor), sizeof(versionMajor));
        os.write(reinterpret_cast<const char *>(&versionBuild), sizeof(versionBuild));
        os.write(reinterpret_cast<const char *>(&versionHotFix), sizeof(versionHotFix));
    }
    return os;
}

using base = system::IfdkStreamer;

const std::string LogStreamer::systemType = "cavs";
const std::string LogStreamer::formatType = "fwlogs";
const int LogStreamer::majorVersion = 1;
const int LogStreamer::minorVersion = 0;

LogStreamer::LogStreamer(Logger &logger, const std::vector<dsp_fw::ModuleEntry> &moduleEntries)
    : base(systemType, formatType, majorVersion, minorVersion), mLogger(logger),
      mModuleEntries(moduleEntries)
{
    /**
     * @todo add properties required by SwAS for IFDK:cavs:fwlog once the SwAS defines them,
     *       using base::addProperty("aKey", "aValue");
     */
}

void LogStreamer::streamFormatHeader(std::ostream &os)
{
    /* Write the number of module entries */
    uint32_t moduleEntriesNb = static_cast<uint32_t>(mModuleEntries.size());
    os.write(reinterpret_cast<const char *>(&moduleEntriesNb), sizeof(moduleEntriesNb));

    /* Write all the module entries */
    os << mModuleEntries;

    /* Flush the header data */
    os.flush();
}

bool LogStreamer::streamNextFormatData(std::ostream &os)
{
    /* Blocking read to get a log block */
    try {
        std::unique_ptr<LogBlock> block(std::move(mLogger.readLogBlock()));
        if (block == nullptr) {
            /* Logger is closed, no more entries */
            return false;
        }
        os << *block;
    } catch (Logger::Exception &e) {

        throw Streamer::Exception(std::string("Fail to read log: ") + e.what());
    }

    /* Log stream is intrinsically endless */
    return true;
}
}
}
