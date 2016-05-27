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
#include "cAVS/Perf.hpp"
#include <sstream>
#include <numeric>

namespace debug_agent
{
namespace core
{

using cavs::dsp_fw::GlobalMemoryState;
using cavs::Perf;

std::set<std::string> SubsystemParameterApplier::getSupportedTypes() const
{
    return {BaseModelConverter::subsystemName};
}

template <class Iterable>
static std::string join(Iterable items)
{
    // join the items like so: "1 2 3 4"...
    return std::accumulate(begin(items), end(items), std::string(),
                           [](std::string acc, typename Iterable::value_type item) {
                               return acc.empty() ? std::to_string(item)
                                                  : (acc + " " + std::to_string(item));
                           });
}

static void printMemoryState(std::ostringstream &out, const GlobalMemoryState::SramStateInfo &state)
{
    out << "        <IntegerParameter Name=\"free_phys_mem_pages\">" << state.freePhysMemPages
        << "</IntegerParameter>\n"
        << "        <IntegerParameter Name=\"ebb_state\">" << join(state.ebbStates)
        << "</IntegerParameter>\n"
        << "        <IntegerParameter Name=\"page_alloc\">" << join(state.pageAlloc)
        << "</IntegerParameter>\n";
}

static void printPerfItems(std::ostringstream &out, const std::vector<Perf::Item> &items)
{
    int index = 0;
    for (const auto &item : items) {
        out << "        <ParameterBlock Name=\"" << index << "\">\n"
            << "            <StringParameter Name=\"uuid\">" << item.uuid << "</StringParameter>\n"
            << "            <IntegerParameter Name=\"instance_id\">" << item.instanceId
            << "</IntegerParameter>\n"
            << "            <StringParameter Name=\"power_mode\">"
            << Perf::powerModeHelper().toString(item.powerMode) << "</StringParameter>\n"
            << "            <BooleanParameter Name=\"is_removed\">" << item.isRemoved
            << "</BooleanParameter>\n"
            << "            <IntegerParameter Name=\"budget\">" << item.budget
            << "</IntegerParameter>\n"
            << "            <IntegerParameter Name=\"peak\">" << item.peak
            << "</IntegerParameter>\n"
            << "            <IntegerParameter Name=\"average\">" << item.average
            << "</IntegerParameter>\n"
            << "        </ParameterBlock>\n";
        ++index;
    }
}

std::string SubsystemParameterApplier::getParameterValue(const std::string & /*type*/,
                                                         ParameterKind kind,
                                                         const std::string &instanceId)
{
    if (kind == ParameterKind::Control) {
        return "<control_parameters/>";
    }

    if (kind != ParameterKind::Info or instanceId != "0") {
        throw UnsupportedException();
    }

    std::ostringstream result;
    result << "<info_parameters>\n";

    {
        auto perfData = mSystem.getPerfService().getData();
        result << "<ParameterBlock Name=\"PerformanceData\">\n"
               << "    <ParameterBlock Name=\"Cores\">\n";
        printPerfItems(result, perfData.cores);
        result << "    </ParameterBlock>\n"
               << "    <ParameterBlock Name=\"Modules\">\n";
        printPerfItems(result, perfData.modules);
        result << "    </ParameterBlock>\n"
               << "</ParameterBlock>\n";
    }

    {
        auto memoryState = mSystem.getModuleHandler().getGlobalMemoryState();
        result << "<ParameterBlock Name=\"MemoryState\">\n"
               << "    <ParameterBlock Name=\"LP\">\n";
        printMemoryState(result, memoryState.lpsramState);
        result << "    </ParameterBlock>\n"
               << "    <ParameterBlock Name=\"HP\">\n";
        printMemoryState(result, memoryState.hpsramState);
        result << "    </ParameterBlock>\n"
               << "</ParameterBlock>\n";
    }

    result << "</info_parameters>\n";

    return result.str();
}
} // core
} // debug_agent
