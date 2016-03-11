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

#include "cAVS/Linux/CompressTypes.hpp"
#include "Poco/String.h"
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

static const std::string infoProcEntry{"info"};

/** Id Info field for a logger compress device shall be as "id: Core x Trace Buffer". */
static const std::string infoIdCore{"Core"};
static const std::string infoIdLogger{"Trace Buffer"};

struct Exception : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

/* This factory creates real compress device */
class AudioProcfsHelper
{
public:
    /** Check and extract the logger compress device information from the info procfs entry of a
     * device.
     * @param[in] infoId literal value of the info id field of the procfs entry
     * @param[out] info retrieved from the info if field for a logger device.
     * @return true if the infoId is matching the compress info requested, false otherwise.
     */
    static bool getCompressDeviceInfo(const std::string &infoId, compress::LoggerInfo &info)
    {
        /* is it a logger device? */
        int rolePos = infoId.find(infoIdLogger);
        if (rolePos == std::string::npos) {
            return false;
        }
        /* it is a logger device, which core for? */
        int corePos = infoId.find(infoIdCore);
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
        return true;
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

            if (getCompressDeviceInfo(infoId, info)) {
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
        int cardId = getCardId(device);
        int deviceId = getDeviceId(device);

        std::string suffix{*device.rbegin()};
        /* @todo remove this once media provides a suffix for compress device as well. */
        if (suffix != "p" && suffix != "c") {
            suffix.clear();
        }
        const std::string deviceProcInfo{soundProcDir + soundCardPrefix + std::to_string(cardId) +
                                         "/" + pcmDevicePrefix + std::to_string(deviceId) + suffix +
                                         "/" + infoProcEntry};
        std::ifstream infoFileStream;
        infoFileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit |
                                  std::ifstream::eofbit);
        try {
            infoFileStream.open(deviceProcInfo, std::ifstream::in);
        } catch (const std::exception &) {
            throw Exception("error while opening " + deviceProcInfo + " file.");
        }
        std::string infoFileIdLine;
        try {
            while (std::getline(infoFileStream, infoFileIdLine)) {
                if (infoFileIdLine.find(field) != std::string::npos) {
                    infoFileIdLine = infoFileIdLine.substr(field.size());
                    break;
                }
                // Not found, reset the line.
                infoFileIdLine.clear();
            }

        } catch (const std::exception &) {
            throw Exception("error during read operation: " + infoFileStream.rdstate());
        }
        try {
            infoFileStream.close();
        } catch (const std::exception &) {
        }
        return infoFileIdLine;
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
