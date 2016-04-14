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

#include "Util/convert.hpp"
#include <tinycompress/tinycompress.h>
#include <sound/compress_params.h>
#include <sound/compress_offload.h>
#include <string.h>
#include <dirent.h>
#include <vector>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace linux
{
namespace compress
{

static const unsigned int defaultChannels = 1;
static const unsigned int defaultRate = 44100;

struct Exception : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

enum class Role
{
    Playback,
    Capture
};

struct Config
{
    Config(size_t fragmentSize, size_t nbFragments)
    {
        mCodec.id = SND_AUDIOCODEC_PCM;
        mCodec.ch_in = defaultChannels;
        mCodec.ch_out = defaultChannels;
        mCodec.sample_rate = defaultRate;
        mCodec.format = SNDRV_PCM_FORMAT_S16_LE;
        mConfig.fragment_size = fragmentSize;
        mConfig.fragments = nbFragments;
        mConfig.codec = &mCodec;
    }
    std::size_t getBufferSize() const { return mConfig.fragment_size * mConfig.fragments; }
    struct compr_config *getConfig() { return &mConfig; }

    struct compr_config mConfig;
    struct snd_codec mCodec;
};

class DeviceInfo
{
public:
    DeviceInfo() = default;
    DeviceInfo(unsigned int cardId, unsigned int deviceId) : mCardId(cardId), mDeviceId(deviceId) {}

    unsigned int cardId() const { return mCardId; }
    unsigned int deviceId() const { return mDeviceId; }
    const std::string name() const
    {
        return {"(" + std::to_string(mCardId) + ":" + std::to_string(mDeviceId) + ")"};
    }

private:
    unsigned int mCardId;
    unsigned int mDeviceId;
};

class LoggerInfo : public DeviceInfo
{
public:
    LoggerInfo() = default;
    LoggerInfo(unsigned int cardId, unsigned int deviceId, unsigned int coreId = 0)
        : DeviceInfo(cardId, deviceId), mCoreId(coreId)
    {
    }
    void setCoreId(unsigned int coreId) { mCoreId = coreId; }
    unsigned int coreId() const { return mCoreId; }

private:
    /**
     * Each compress device for logging is providing to a given DSP core.
     * mCoreId stores the ID of the core corresponding to this device.
     */
    unsigned int mCoreId;
};

class InjectionProbeInfo : public DeviceInfo
{
public:
    InjectionProbeInfo() = default;
    InjectionProbeInfo(unsigned int cardId, unsigned int deviceId, std::size_t probeIndex = 0)
        : DeviceInfo(cardId, deviceId), mProbeIndex(probeIndex)
    {
    }
    void setProbeIndex(unsigned int probeIndex) { mProbeIndex = probeIndex; }
    std::size_t probeIndex() const { return mProbeIndex; }

private:
    /**
     * Each compress device for probe injection is dedicated to a probe index or id.
     */
    std::size_t mProbeIndex;
};

template <Role role>
class ProbeInfo : public DeviceInfo
{
public:
    ProbeInfo(unsigned int cardId, unsigned int deviceId) : DeviceInfo(cardId, deviceId) {}
};

using ExtractionProbeInfo = ProbeInfo<Role::Capture>;
using InjectionProbesInfo = std::vector<InjectionProbeInfo>;
using DevicesInfo = std::vector<DeviceInfo>;
using LoggersInfo = std::vector<LoggerInfo>;
}
}
}
}
