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

#include "cAVS/Linux/CompressTypes.hpp"
#include "Poco/String.h"
#include "Poco/StringTokenizer.h"
#include <stdexcept>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

static const std::string soundCardPrefix{"card"};
static const std::string compressDevicePrefix{"compr"};
static const std::string pcmDevicePrefix{"pcm"};
static const std::string infoId{"id:"};
static const std::string soundDeviceDir{"/dev/snd"};
static const std::string soundProcDir{"/proc/asound/"};
static const std::string devicesProcfsEntry{"/proc/asound/devices"};
static const std::string infoProcEntry{"info"};

/** Id Info field for a logger compress device shall be as "id: Core x Trace Buffer". */
static const std::string infoIdCore{"Core"};

template <typename info>
struct DeviceInfoTrait;

template <>
struct DeviceInfoTrait<compress::ExtractionProbeInfo>
{
    static const std::string mTag;
};
template <>
struct DeviceInfoTrait<compress::InjectionProbeInfo>
{
    static const std::string mTag;
};
template <>
struct DeviceInfoTrait<compress::LoggerInfo>
{
    static const std::string mTag;
};
const std::string DeviceInfoTrait<compress::LoggerInfo>::mTag{"Trace Buffer"};
const std::string DeviceInfoTrait<compress::ExtractionProbeInfo>::mTag{"Probe Capture"};
const std::string DeviceInfoTrait<compress::InjectionProbeInfo>::mTag{"Probe Playback"};

struct Exception : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/* This factory creates real compress device */
class AudioProcfsHelper
{
public:
    /** Procfs /proc/asound/devices entry exposes the list of device and their associated type
     * e.g. control, playback, capture.
     * This helper function returns the name of the device of the given type.
     */
    static const std::string getDeviceType(const std::string &type)
    {
        std::string deviceTypeLine{getLineWithTagFromFile(devicesProcfsEntry, type)};
        if (!deviceTypeLine.empty()) {
            Poco::StringTokenizer tokenize(deviceTypeLine, "[]", Poco::StringTokenizer::TOK_TRIM);
            if (tokenize.count() > 1) {
                return tokenize[1];
            }
        }
        return {};
    }

    /**
     * Fallback for any compress device that does not provides any extra useful information
     * from procfs.
     */
    static void getCompressDeviceInfo(const std::string &, compress::DeviceInfo &) {}

    /** Extract the specific logger compress device information from the info procfs entry of a
     * device.
     * @param[in] infoId literal value of the info id field of the procfs entry
     * @param[out] info retrieved from the info if field for a logger device.
     */
    static void getCompressDeviceInfo(const std::string &infoId, compress::LoggerInfo &info)
    {
        std::string::size_type rolePos{infoId.find(DeviceInfoTrait<compress::LoggerInfo>::mTag)};
        /* it is a logger device, which core for? */
        std::string::size_type corePos = infoId.find(infoIdCore);
        if (corePos == std::string::npos) {
            throw Exception("Compress Device found for Log without Core assigned.");
        }
        const std::string coreLiteral{
            infoId.substr(corePos + infoIdCore.size(), rolePos - corePos - infoIdCore.size())};
        unsigned int coreId;
        if (!convertTo(Poco::trim(coreLiteral), coreId)) {
            throw Exception("Invalid core id found in info " + infoId);
        }
        info.setCoreId(coreId);
    }

    /** Find and extract the information of a given type of compress device available on the
     * platform. it relies on the information read from /dev and /proc.
     * @tparam CompressInfo type of compress information requested (may be logger, prober)
     * @param[in] infoId literal value of the info id field of the procfs entry
     * @param[out] info retrieved from the info id field.
     * @return vector of CompressInfo.
     */
    template <typename CompressInfo>
    std::vector<CompressInfo> getCompressDeviceInfoList() const
    {
        struct dirent **fileList;
        int count = scandir(soundDeviceDir.c_str(), &fileList, compressDeviceFilter, NULL);
        if (count <= 0) {
            throw Exception("No compressed devices found for logging.");
        }
        std::vector<CompressInfo> compressInfoList;
        for (int index = 0; index < count; index++) {
            const std::string deviceName = fileList[index]->d_name;
            CompressInfo info{getCardId(deviceName), getDeviceId(deviceName)};

            /* lets retrieve information from procfs for this device. */
            std::string infoId{getDeviceIdInfo(deviceName)};
            auto &infoTag = DeviceInfoTrait<CompressInfo>::mTag;
            if (infoId.find(infoTag) != std::string::npos) {
                getCompressDeviceInfo(infoId, info);
                compressInfoList.push_back(info);
            }
        }
        return compressInfoList;
    }

    /** Compress device directory filter function.
     * @param[out] entry found in the directory parsed matching with compress device prefix.
     * @return 1 if at least one compress device has been found, 0 otherwise.
     */
    static int compressDeviceFilter(const struct dirent *entry)
    {
        return prefixFilter(entry, compressDevicePrefix);
    }

    /** generic directory filter function.
     * @param[out] entry found in the directory parsed matching with compress device prefix.
     * @param[in] prefix to search for
     * @return 1 if at least one compress device has been found, 0 otherwise.
     */
    static int prefixFilter(const struct dirent *entry, const std::string &prefix)
    {
        std::string entryName(entry->d_name);
        return entryName.find(prefix) != std::string::npos;
    }

    /** Extract the card Id from a device name e.g. comprCxDy{p|c|} or pcmCxDy{p|c}
     * @param[in] deviceName literal name of a sound device
     * @return card index if found
     * @throw CompressDeviceFactory::Exception if failed to extract card index
     */
    static unsigned int getCardId(const std::string &deviceName)
    {
        /* Extract a substring between devicePrefix + "C" and "D". */
        size_t positionC = deviceName.find("C");
        size_t positionD = deviceName.find("D");
        if (positionC == std::string::npos || positionD == std::string::npos) {
            throw Exception("Invalid card.");
        }
        const std::string cardIdLiteral{
            deviceName.substr(positionC + 1, positionD - positionC - 1)};
        int cardId;
        if (!convertTo(cardIdLiteral, cardId)) {
            throw Exception("Invalid card.");
        }
        return cardId;
    }

    /** Extract the device Id from a device name e.g. comprCxDy{p|c|} or pcmCxDy{p|c}
     * @param[in] deviceName literal name of a sound device
     * @return device index if found
     * @throw CompressDeviceFactory::Exception if failed to extract device index
     */
    static unsigned int getDeviceId(const std::string &deviceName)
    {
        size_t positionD = deviceName.find("D");
        if (positionD == std::string::npos) {
            throw Exception("Invalid device.");
        }
        /* Extract a substring between devicePrefix + "Cx" + "D" and null or "p" or "c". */
        std::string deviceIdLiteral{deviceName.substr(positionD + 1)};
        if (deviceIdLiteral.find("p") != std::string::npos ||
            deviceIdLiteral.find("c") != std::string::npos) {
            /* we shall remove the playback / capture letter suffix. */
            deviceIdLiteral.resize(deviceIdLiteral.size() - 1);
        }
        int deviceId;
        if (!convertTo(deviceIdLiteral, deviceId)) {
            throw Exception("Invalid compress device.");
        }
        return deviceId;
    }

    /** Extract the requested field of "info" procfs entry for a given sound device.
     * @param[in] device literal name of a sound device
     * @param[in] field to be retrieve from "info" entry
     * @return literal value of the requested field for the requested device.
     * @throw CompressDeviceFactory::Exception if failed to extract device index
     */
    static const std::string getDeviceInfoField(const std::string &device, const std::string &field)
    {
        size_t positionC = device.find("C");
        if (positionC == std::string::npos) {
            throw Exception("Invalid device name, shall have reference to Card.");
        }
        const std::string devicePrefix{device.substr(0, positionC)};
        int cardId = getCardId(device);
        int deviceId = getDeviceId(device);

        std::string suffix{*device.rbegin()};
        /* @todo remove this once media provides a suffix for compress device as well. */
        if (suffix != "p" && suffix != "c") {
            suffix.clear();
        }
        const std::string deviceProcInfo{soundProcDir + soundCardPrefix + std::to_string(cardId) +
                                         "/" + devicePrefix + std::to_string(deviceId) + suffix +
                                         "/" + infoProcEntry};

        std::string lineWithInfoField = getLineWithTagFromFile(deviceProcInfo, field);
        if (!lineWithInfoField.empty()) {
            // Removes "id:" from the line.
            lineWithInfoField = lineWithInfoField.substr(field.size());
        }
        return lineWithInfoField;
    }

    static const std::string getLineWithTagFromFile(const std::string &fileName,
                                                    const std::string &tag)
    {
        std::ifstream infoFileStream;
        infoFileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit |
                                  std::ifstream::eofbit);
        try {
            infoFileStream.open(fileName, std::ifstream::in);
        } catch (const std::exception &) {
            throw Exception("error while opening " + fileName + " file.");
        }
        std::string lineWithTag;
        try {
            while (std::getline(infoFileStream, lineWithTag)) {
                if (lineWithTag.find(tag) != std::string::npos) {
                    break;
                }
                // Not found, reset the line.
                lineWithTag.clear();
            }
        } catch (const std::exception &) {
            throw Exception("error during read operation: " + infoFileStream.rdstate());
        }
        try {
            infoFileStream.close();
        } catch (const std::exception &) {
        }
        return lineWithTag;
    }

    /** Extract the "id" field of "info" procfs entry for a given sound device.
     * @param[in] device literal name of a sound device
     * @return literal value of the "id" field for the requested device.
     * @throw CompressDeviceFactory::Exception if failed to extract device index
     */
    static const std::string getDeviceIdInfo(const std::string &deviceName)
    {
        return getDeviceInfoField(deviceName, infoId);
    }
};
}
}
}
