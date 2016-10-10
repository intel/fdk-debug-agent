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

#include "cAVS/DspFw/Probe.hpp"
#include "cAVS/DspFw/Common.hpp"
#include "cAVS/ModuleHandler.hpp"

namespace debug_agent
{
namespace cavs
{
namespace linux
{
namespace mixer_ctl
{

static const std::string logLevelMixer{"DSP Log Level"};

/**
 * Log Mixer Ctl levels, beware of the type, it shall match the mixer ctl value type
 * Log Level is an enum control mixer, type is int
 */
enum class LogPriority : uint32_t
{
    Quiet = 0,
    Critical,
    High,
    Medium,
    Low,
    Verbose
};

enum class ProbeState : uint8_t
{
    Connect = private_fw::dsp_fw::PROBE_POINTS,
    Disconnect = private_fw::dsp_fw::DISCONNECT_PROBE_POINTS
};

enum class ProbePurpose : uint32_t
{
    Extract = private_fw::dsp_fw::EXTRACT,
    Inject = private_fw::dsp_fw::INJECT,
    InjectReextract = private_fw::dsp_fw::INJECT_REEXTRACT
};

const std::string mExtractorControlTag{"Probe probe 0 Extractor"};
const std::string mInjectorControlTag{"Probe probe 0 Injector"};

static inline const std::string getProbeExtractControl(size_t index)
{
    return {mExtractorControlTag + std::to_string(index) + " params"};
}

static inline const std::string getProbeInjectControl(size_t index)
{
    return {mInjectorControlTag + std::to_string(index) + " params"};
}

class ProbeControl
{
public:
    ProbeControl() = default;
    ProbeControl(ProbeState state, ProbePurpose purpose, dsp_fw::ProbePointId point)
        : mState(static_cast<uint8_t>(state)), mPurpose(purpose), mPointId(point)
    {
    }

    void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(mState);
        reader.read(mPurpose);
        reader.read(mPointId);
    }

    void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(mState);
        writer.write(mPurpose);
        writer.write(mPointId);
    }
    ProbeState getState() const { return static_cast<ProbeState>(mState); }
    ProbePurpose getPurpose() const { return mPurpose; }
    dsp_fw::ProbePointId getPointId() const { return mPointId; }

private:
    uint8_t mState; /**< Probe state is encoded on one byte for mixer control. */
    ProbePurpose mPurpose;
    dsp_fw::ProbePointId mPointId; /**< ProbeID is directly mapped on FW IPC structure. */
};
}
}
}
}
