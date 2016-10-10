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
