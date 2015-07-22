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

    uint32_t moduleId = 0;
    for (auto moduleEntry : moduleEntries) {
        /* The Module ID must be streamed out as 32bits word little endian.
         * The module ID is not part of the ModuleEntry struct: it corresponds to the index of the
         * ModuleEntry in the ModuleEntry vector */
        os.write(reinterpret_cast<char *>(&moduleId), sizeof(moduleId));
        moduleId++;

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

LogStreamer::LogStreamer(Logger &logger, const std::vector<dsp_fw::ModuleEntry> &moduleEntries):
    base(systemType, formatType, majorVersion, minorVersion),
    mLogger(logger),
    mModuleEntries(moduleEntries)
{
    /**
     * @todo add properties required by SwAS for IFDK:cavs:fwlog once the SwAS defines them,
     *       using base::addProperty("aKey", "aValue");
     */
}

void LogStreamer::streamFormatHeader(std::ostream &os)
{
    os << mModuleEntries;
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
    }
    catch (Logger::Exception &e) {

        throw Streamer::Exception(std::string("Fail to read log: ") + e.what());
    }

    /* Log stream is intrinsically endless */
    return true;
}

}
}
