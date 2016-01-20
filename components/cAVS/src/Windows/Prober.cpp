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

#include "cAVS/Windows/Prober.hpp"

namespace debug_agent
{
namespace cavs
{
namespace windows
{

void Prober::setState(State state)
{
    /* TO DO */
}

Prober::State Prober::getState()
{
    /* TO DO */
    return State::Idle;
}

void Prober::setSessionProbes(const std::vector<ProbeConfig> probes)
{
    /* TO DO */
}

std::vector<Prober::ProbeConfig> Prober::getSessionProbes()
{
    /* TO DO */
    return std::vector<ProbeConfig>();
}

std::unique_ptr<util::Buffer> Prober::dequeueExtractionBlock(uint32_t probeIndex)
{
    /* TO DO */
    return nullptr;
}

bool Prober::enqueueInjectionBlock(uint32_t probeIndex, const util::Buffer &buffer)
{
    /* TO DO */
    return false;
}
}
}
}
