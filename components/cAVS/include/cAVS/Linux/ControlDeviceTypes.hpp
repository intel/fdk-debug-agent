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
    Connect = 3,
    Disconnect = 4
};
enum class ProbePurpose : uint32_t
{
    Inject,
    Extract,
    InjectReextract
};

static inline const std::string getProbeExtractControlMixer(size_t index)
{
    return {"Probe probe 0 Extractor" + std::to_string(index) + " params"};
}

static inline const std::string getProbeInjectControlMixer(size_t index)
{
    return {"Probe probe 0 Injector" + std::to_string(index) + " params"};
}

static constexpr uint32_t maxProbes = 8;

class ProbeControl
{
public:
    ProbeControl() = default;
    ProbeControl(ProbeState state, ProbePurpose purpose, dsp_fw::ProbePointId point)
        : mState(state), mPurpose(purpose), mPointId(point)
    {
    }

    virtual void fromStream(util::ByteStreamReader &reader)
    {
        reader.read(mState);
        reader.read(mPurpose);
        mPointId.fromStream(reader);
    }

    virtual void toStream(util::ByteStreamWriter &writer) const
    {
        writer.write(mState);
        writer.write(mPurpose);
        mPointId.toStream(writer);
    }
    ProbeState getState() const { return mState; }
    ProbePurpose getPurpose() const { return mPurpose; }
    dsp_fw::ProbePointId getPointId() const { return mPointId; }

private:
    ProbeState mState;
    ProbePurpose mPurpose;
    dsp_fw::ProbePointId mPointId; /**< ProbeID is directly mapped on FW IPC structure. */
};
}
}
}
}
