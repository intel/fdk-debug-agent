/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
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
#include "cAVS/Topology.hpp"
#include <string>

namespace debug_agent
{
namespace cavs
{

const DSModuleInstanceProps &Topology::getModuleInstance(ModuleCompoundId moduleInstanceId) const
{
    const auto it = moduleInstances.find(moduleInstanceId);
    if (it == moduleInstances.end()) {

        uint16_t moduleId;
        uint16_t instanceId;

        splitModuleInstanceId(moduleInstanceId, moduleId, instanceId);

        throw Exception("Topology inconsistency: undefined module instance "
                + std::to_string(instanceId) + " of module ID" + std::to_string(moduleId));
    }

    return it->second;
}

void Topology::addAllModuleOutputs(OutputList &list, ModuleCompoundId module) const
{
    const DSModuleInstanceProps &moduleProps = getModuleInstance(module);
    for (OutputId outputId = 0;
         outputId < moduleProps.output_pins.pin_info.size();
         ++outputId) {

        list.push_back(Output(module, outputId));
    }
}

void Topology::addAllModuleInputs(InputList &list, ModuleCompoundId module) const
{
    const DSModuleInstanceProps &moduleProps = getModuleInstance(module);
    for (InputId inputId = 0;
         inputId < moduleProps.input_pins.pin_info.size();
         ++inputId) {

        list.push_back(Input(module, inputId));
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
        for (size_t pipeModuleIndex = 0;
             pipeModuleIndex < pipe.module_instances.size() - 1;
             ++pipeModuleIndex) {

                ModuleCompoundId sourceModuleId = pipe.module_instances[pipeModuleIndex];
                ModuleCompoundId destinationModuleId = pipe.module_instances[pipeModuleIndex + 1];

                computeModulesPairLink(sourceModuleId,
                                       destinationModuleId,
                                       unresolvedInputs,
                                       unresolvedOutputs);
        }
    }
}

void Topology::computeModulesPairLink(ModuleCompoundId sourceModuleId,
                                      ModuleCompoundId destinationModuleId,
                                      InputList &unresolvedInputs,
                                      OutputList &unresolvedOutputs)
{
    /* List all destination module outputs */
    OutputList sourceOutputs;
    addAllModuleOutputs(sourceOutputs, sourceModuleId);

    /* List all destination module inputs */
    InputList destinationInputs;
    addAllModuleInputs(destinationInputs, destinationModuleId);

    /* For each output of source module... */
    for (Output const &output : sourceOutputs) {

        const DSModuleInstanceProps &sourceModule = getModuleInstance(sourceModuleId);
        const dsp_fw::PinProps &outputPin = sourceModule.output_pins.pin_info[output.second];
        bool connectionFound = false;

        /* ...look for an input connected to the same queue */
        for (auto inputIt = destinationInputs.begin();
             inputIt != destinationInputs.end(); ) {

            const DSModuleInstanceProps &destinationModule = getModuleInstance(destinationModuleId);
            const dsp_fw::PinProps &inputPin =
                destinationModule.input_pins.pin_info[inputIt->second];

            if (outputPin.phys_queue_id == inputPin.phys_queue_id) {

                /* Output and input are connected to the same queue: we've just found a link */
                links.push_back(Link(output.first, output.second,
                                     inputIt->first, inputIt->second));

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
    unresolvedInputs.insert(unresolvedInputs.end(),
                            destinationInputs.begin(),
                            destinationInputs.end());
}

void Topology::computeInterPipeLinks(InputList &unresolvedInputs, OutputList &unresolvedOutputs)
{
    for (auto const &output : unresolvedOutputs) {

        const DSModuleInstanceProps &sourceModule = getModuleInstance(output.first);
        const dsp_fw::PinProps &outputPinProps = sourceModule.output_pins.pin_info[output.second];

        for (auto const &input : unresolvedInputs) {

            const DSModuleInstanceProps &destinationModule = getModuleInstance(input.first);
            const dsp_fw::PinProps &inputPinProps =
                destinationModule.input_pins.pin_info[input.second];

            if (outputPinProps.phys_queue_id == inputPinProps.phys_queue_id) {

                links.push_back(Link(output.first, output.second,
                                     input.first, input.second));
                break;
            }
        }
    }
}

}
}


