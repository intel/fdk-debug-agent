/*
 * Copyright (c) 2015, Intel Corporation
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
#include "cAVS/Topology.hpp"
#include <string>
#include <iostream>

namespace debug_agent
{
namespace cavs
{

const dsp_fw::ModuleInstanceProps &Topology::getModuleInstance(
    const dsp_fw::CompoundModuleId &moduleInstanceId) const
{
    const auto it = moduleInstances.find(moduleInstanceId);
    if (it == moduleInstances.end()) {
        throw Exception("Topology inconsistency: undefined module instance " +
                        std::to_string(moduleInstanceId.instanceId) + " of module ID" +
                        std::to_string(moduleInstanceId.moduleId));
    }

    return it->second;
}

void Topology::addAllModuleOutputs(OutputList &list, const dsp_fw::CompoundModuleId &module) const
{
    const dsp_fw::ModuleInstanceProps &moduleProps = getModuleInstance(module);

    OutputId outputId;
    if (moduleProps.output_gateway.val.dw == dsp_fw::ConnectorNodeId::kInvalidNodeId) {

        outputId = 0;
    } else {

        /* There is a gateway connected to pin id #0: ignore this pin in the next loop */
        outputId = 1;
    }

    for (; outputId < moduleProps.output_pins.pin_info.size(); ++outputId) {

        if (moduleProps.output_pins.pin_info[outputId].phys_queue_id !=
            dsp_fw::PinProps::invalidQueueId) {

            list.push_back(Output(module, outputId));
        }
    }
}

void Topology::addAllModuleInputs(InputList &list, const dsp_fw::CompoundModuleId &module) const
{
    const dsp_fw::ModuleInstanceProps &moduleProps = getModuleInstance(module);

    InputId inputId;
    if (moduleProps.input_gateway.val.dw == dsp_fw::ConnectorNodeId::kInvalidNodeId) {

        inputId = 0;
    } else {

        /* There is a gateway connected to pin id #0: ignore this pin in the next loop */
        inputId = 1;
    }

    for (; inputId < moduleProps.input_pins.pin_info.size(); ++inputId) {

        if (moduleProps.input_pins.pin_info[inputId].phys_queue_id !=
            dsp_fw::PinProps::invalidQueueId) {

            list.push_back(Input(module, inputId));
        }
    }
}

void Topology::computeLinks()
{
    links.clear();

    /* The unresolved input and output list which will be used during link computation */
    InputList unresolvedInputs;
    OutputList unresolvedOutputs;

    /* Look for intra pipe links */
    computeIntraPipeLinks(unresolvedInputs, unresolvedOutputs);

    /* Look for inter pipe links */
    computeInterPipeLinks(unresolvedInputs, unresolvedOutputs);

    /* unresolvedInputs and unresolvedOutputs shall now be empty */
    checkUnresolved(unresolvedInputs, unresolvedOutputs);
}

void Topology::computeIntraPipeLinks(InputList &unresolvedInputs, OutputList &unresolvedOutputs)
{
    for (auto const &pipe : pipelines) {

        if (pipe.module_instances.size() == 0) {

            // No module instance in this pipe: continue with next one
            continue;
        }
        /* In a pipe, module instances collection is ordered like the module are ordered:
         * Gather first module instance inputs into unresolved inputs connections list
         */
        addAllModuleInputs(unresolvedInputs, pipe.module_instances.front());

        /* Gather last module instance outputs into unresolved outputs connections list */
        addAllModuleOutputs(unresolvedOutputs, pipe.module_instances.back());

        if (pipe.module_instances.size() < 2) {

            // Nothing more to be done if the pipe contains only one module
            continue;
        }

        /* Now search for links between modules within the current pipe */
        for (size_t pipeModuleIndex = 0; pipeModuleIndex < pipe.module_instances.size() - 1;
             ++pipeModuleIndex) {

            const dsp_fw::CompoundModuleId &sourceModuleId = pipe.module_instances[pipeModuleIndex];
            const dsp_fw::CompoundModuleId &destinationModuleId =
                pipe.module_instances[pipeModuleIndex + 1];

            computeModulesPairLink(sourceModuleId, destinationModuleId, unresolvedInputs,
                                   unresolvedOutputs);
        }
    }
}

void Topology::computeModulesPairLink(const dsp_fw::CompoundModuleId &sourceModuleId,
                                      const dsp_fw::CompoundModuleId &destinationModuleId,
                                      InputList &unresolvedInputs, OutputList &unresolvedOutputs)
{
    /* List all destination module outputs */
    OutputList sourceOutputs;
    addAllModuleOutputs(sourceOutputs, sourceModuleId);

    /* List all destination module inputs */
    InputList destinationInputs;
    addAllModuleInputs(destinationInputs, destinationModuleId);

    /* For each output of source module... */
    for (Output const &output : sourceOutputs) {

        const dsp_fw::ModuleInstanceProps &sourceModule = getModuleInstance(sourceModuleId);
        const dsp_fw::PinProps &outputPin = sourceModule.output_pins.pin_info[output.second];
        bool connectionFound = false;

        /* ...look for an input connected to the same queue */
        for (auto inputIt = destinationInputs.begin(); inputIt != destinationInputs.end();) {

            const dsp_fw::ModuleInstanceProps &destinationModule =
                getModuleInstance(destinationModuleId);
            const dsp_fw::PinProps &inputPin =
                destinationModule.input_pins.pin_info[inputIt->second];

            if (outputPin.phys_queue_id == inputPin.phys_queue_id) {

                /* Output and input are connected to the same queue: we've just found a link */
                links.push_back(Link(output.first, output.second, inputIt->first, inputIt->second));

                // Remind the output has a connection with an input
                connectionFound = true;

                /* This input is used: remove it from the list */
                destinationInputs.erase(inputIt);
                break;
            } else {

                /* Try next input */
                ++inputIt;
            }
        }

        if (!connectionFound) {

            /* This output is not connected to any input: unresolved */
            unresolvedOutputs.push_back(output);
        }
    }

    /* Remaining inputs of the destination module are unresolved */
    unresolvedInputs.insert(unresolvedInputs.end(), destinationInputs.begin(),
                            destinationInputs.end());
}

void Topology::computeInterPipeLinks(InputList &unresolvedInputs, OutputList &unresolvedOutputs)
{
    bool connectionFound;
    for (auto outputIt = unresolvedOutputs.begin(); outputIt != unresolvedOutputs.end();) {

        connectionFound = false;

        const dsp_fw::ModuleInstanceProps &sourceModule = getModuleInstance(outputIt->first);
        const dsp_fw::PinProps &outputPinProps =
            sourceModule.output_pins.pin_info[outputIt->second];

        for (auto inputIt = unresolvedInputs.begin(); inputIt != unresolvedInputs.end();) {

            const dsp_fw::ModuleInstanceProps &destinationModule =
                getModuleInstance(inputIt->first);
            const dsp_fw::PinProps &inputPinProps =
                destinationModule.input_pins.pin_info[inputIt->second];

            if (outputPinProps.phys_queue_id == inputPinProps.phys_queue_id) {

                connectionFound = true;
                links.push_back(
                    Link(outputIt->first, outputIt->second, inputIt->first, inputIt->second));

                unresolvedInputs.erase(inputIt);
                break;
            } else {

                ++inputIt;
            }
        }

        if (connectionFound) {

            outputIt = unresolvedOutputs.erase(outputIt);
        } else {

            ++outputIt;
        }
    }
}

void Topology::checkUnresolved(InputList &unresolvedInputs, OutputList &unresolvedOutputs) const
{
    for (auto const &output : unresolvedOutputs) {

        const dsp_fw::ModuleInstanceProps &moduleProps = getModuleInstance(output.first);

        /** @fixme use cAVS plugin log instead */
        std::cout << "[cAVS] Error: "
                  << "Unconnected output pin #" << output.second << " of module instance ID #"
                  << output.first.instanceId << " (Module ID #" << output.first.moduleId
                  << "): expecting connection through queue ID #"
                  << moduleProps.output_pins.pin_info[output.second].phys_queue_id << std::endl;
    }
    for (auto const &input : unresolvedInputs) {

        const dsp_fw::ModuleInstanceProps &moduleProps = getModuleInstance(input.first);

        /** @fixme use cAVS plugin log instead */
        std::cout << "[cAVS] Error: "
                  << "Unconnected input pin #" << input.second << " of module instance ID #"
                  << input.first.instanceId << " (Module ID #" << input.first.moduleId
                  << "): expecting connection through queue ID #"
                  << moduleProps.input_pins.pin_info[input.second].phys_queue_id << std::endl;
    }
}
}
}
