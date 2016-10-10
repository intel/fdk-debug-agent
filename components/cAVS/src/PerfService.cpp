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

#include "cAVS/PerfService.hpp"
#include "Util/Uuid.hpp"

namespace debug_agent
{
namespace cavs
{

PerfService::PerfService(Perf &perf, ModuleHandler &moduleHandler)
    : mPerf(perf), mModuleHandler(moduleHandler)
{
}

PerfService::~PerfService()
{
    try {
        mPerf.setState(Perf::State::Disabled);
    } catch (Exception &e) {
        /* @todo : use log */
        std::cout << "Unable to stop driver perf service: " << e.what() << "\n";
    }
}

Perf::State PerfService::getState()
{
    return mPerf.getState();
}

void PerfService::setState(Perf::State state)
{
    mPerf.setState(state);
}

static float computeBudget(const dsp_fw::ModuleInstanceProps &props)
{
    auto &format = props.input_pins.pin_info[0].format;
    // Check if there can be any runtime overflow with the computation below.
    static_assert(double(std::numeric_limits<decltype(props.cpc)>::max()) *
                          std::numeric_limits<decltype(format.sampling_frequency)>::max() *
                          std::numeric_limits<decltype(format.number_of_channels)>::max() *
                          std::numeric_limits<decltype(format.bit_depth)>::max() <=
                      std::numeric_limits<float>::max(),
                  "Potential floating-point overflow at runtime.");
    if (format.bit_depth % 8 != 0) {
        throw std::range_error("The bit depth for module instance (" +
                               std::to_string(props.id.moduleId) + ", " +
                               std::to_string(props.id.instanceId) + ") is not a multiple of 8.");
    }
    return (float(props.cpc) * format.sampling_frequency * format.number_of_channels *
            (format.bit_depth / 8) / props.ibs_bytes) /
           1000;
}

PerfService::CompoundPerfData PerfService::getData()
{
    CompoundPerfData result;

    try {
        auto raw = mModuleHandler.getPerfItems();

        // Compute the budget for each module instance
        for (const auto &rawItem : raw) {
            bool isCore = rawItem.resourceId.moduleId == 0;
            float budget = 0;
            std::string uuidRepr = "n/a"; // irrelevant for cores

            // The budget for cores defaults to 0
            if (not isCore) {
                if (not rawItem.details.bits.isRemoved) {
                    try {
                        dsp_fw::ModuleInstanceProps props = mModuleHandler.getModuleInstanceProps(
                            rawItem.resourceId.moduleId, rawItem.resourceId.instanceId);
                        budget = computeBudget(props);
                    } catch (ModuleHandler::Exception &e) {
                        // continue on error (leave the budget uncomputed)
                        std::cout << "Couldn't retrieve module instance ("
                                  << rawItem.resourceId.moduleId << ", "
                                  << rawItem.resourceId.instanceId
                                  << ") when trying to compute its KCPS budget: " << e.what();
                    }

                    if (budget > std::numeric_limits<decltype(Perf::Item::budget)>::max()) {
                        throw Exception("Budget kCPS computation overflow.");
                    }
                }

                util::Uuid uuid;
                try {
                    auto moduleEntry = mModuleHandler.findModuleEntry(rawItem.resourceId.moduleId);
                    uuid.fromOtherUuidType(moduleEntry.uuid);
                } catch (ModuleHandler::Exception &e) {
                    throw Exception("When trying to find module entry " +
                                    std::to_string(rawItem.resourceId.moduleId) + ": " +
                                    std::string(e.what()));
                }
                uuidRepr = uuid.toString();
            }

            Perf::Item item{
                uuidRepr,
                rawItem.resourceId.instanceId,
                (rawItem.details.bits.powerMode == 0 ? Perf::PowerMode::D0 : Perf::PowerMode::D0i3),
                bool(rawItem.details.bits.isRemoved),
                decltype(Perf::Item::budget)(budget),
                rawItem.peak,
                rawItem.average};
            // Separate modules and cores, each kind in its own list.
            if (isCore) {
                result.cores.push_back(item);
            } else {
                result.modules.push_back(item);
            }
        }
    } catch (ModuleHandler::Exception &e) {
        throw Exception("When getting perf items from firmware: " + std::string(e.what()));
    }

    return result;
}
} // namespace cavs
} // namespace debug_agent
