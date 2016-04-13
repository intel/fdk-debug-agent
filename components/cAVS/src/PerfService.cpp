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

#include "cAVS/PerfService.hpp"

namespace debug_agent
{
namespace cavs
{
PerfService::PerfService(Perf &perf, ModuleHandler &moduleHandler)
    : mPerf(perf), mModuleHandler(moduleHandler)
{
}

// TODO: this setter is needed because the System constructor can't create the PerfService with a
// correct number of max items. We will be able to fix this if/when the ModuleHandler returns
// results by value instead of via output arguments.
void PerfService::setMaxItemCount(uint32_t maxItemCount)
{
    mMaxItemCount = maxItemCount;
}

Perf::State PerfService::getState()
{
    return mPerf.getState();
}

void PerfService::setState(Perf::State state)
{
    mPerf.setState(state);
}

std::string PerfService::getData()
{
    return "";
}
} // namespace cavs
} // namespace debug_agent
