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
