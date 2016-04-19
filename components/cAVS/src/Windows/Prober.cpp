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
#include "cAVS/Windows/Prober.hpp"
#include <Util/AssertAlways.hpp>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

Prober::~Prober()
{
    mStateMachine.stopNoThrow();
}

void Prober::setState(bool active)
{
    try {
        mStateMachine.setState(active);
    } catch (ProberStateMachine::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}

bool Prober::isActive()
{
    try {
        return mStateMachine.isActive();
    } catch (ProberStateMachine::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}

std::unique_ptr<util::Buffer> Prober::dequeueExtractionBlock(ProbeId probeIndex)
{
    try {
        return mBackend.dequeueExtractionBlock(probeIndex);
    } catch (ProberBackend::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}

bool Prober::enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer)
{
    try {
        return mBackend.enqueueInjectionBlock(probeIndex, buffer);
    } catch (ProberBackend::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}

std::size_t Prober::getMaxProbeCount() const
{
    return mBackend.getMaxProbeCount();
}

void Prober::setProbesConfig(const SessionProbes &probes,
                             const InjectionSampleByteSizes &injectionSampleByteSizes)
{
    try {
        mBackend.setSessionProbes(probes, injectionSampleByteSizes);
    } catch (ProberBackend::Exception &e) {
        throw Exception(std::string(e.what()));
    }
}
}
}
}
