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

#include "Core/SubsystemParameterApplier.hpp"
#include "Core/BaseModelConverter.hpp"
#include "cAVS/ModuleHandler.hpp"
#include <sstream>
#include <numeric>

namespace debug_agent
{
namespace core
{

using cavs::dsp_fw::GlobalMemoryState;

std::set<std::string> SubsystemParameterApplier::getSupportedTypes() const
{
    return {BaseModelConverter::subsystemName};
}

static void printMemoryState(std::ostringstream &out, const GlobalMemoryState::SramStateInfo &state)
{
    out << "        <IntegerParameter Name=\"free_phys_mem_pages\">" << state.freePhysMemPages
        << "</IntegerParameter>\n"
        << "        <IntegerParameter Name=\"ebb_state\">";

    // join the EBB states like so: "1 2 3 4"...
    std::string joinedEbbStates = std::accumulate(
        begin(state.ebbStates), end(state.ebbStates), std::string(),
        [](std::string acc, uint32_t ebbState) {
            return acc.empty() ? std::to_string(ebbState) : (acc + " " + std::to_string(ebbState));
        });

    out << joinedEbbStates << "</IntegerParameter>\n";

    // TODO: page_alloc (once available from the firmware)
}
std::string SubsystemParameterApplier::getParameterValue(const std::string & /*type*/,
                                                         ParameterKind kind,
                                                         const std::string &instanceId)
{
    if (kind != ParameterKind::Info or instanceId != "0") {
        throw UnsupportedException();
    }

    auto memoryState = mSystem.getModuleHandler().getGlobalMemoryState();

    std::ostringstream result;
    // clang-format off
    result << "<info_parameters>\n"
           << "<ParameterBlock Name=\"MemoryState\">\n"
           << "    <ParameterBlock Name=\"LP\">\n";
    printMemoryState(result, memoryState.lpsramState);
    result << "    </ParameterBlock>\n"
           << "    <ParameterBlock Name=\"HP\">\n";
    printMemoryState(result, memoryState.hpsramState);
    result << "    </ParameterBlock>\n"
           << "</ParameterBlock>\n"
           << "</info_parameters>\n";
    // clang-format on
    return result.str();
}
} // core
} // debug_agent
