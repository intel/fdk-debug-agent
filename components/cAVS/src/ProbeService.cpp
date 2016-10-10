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
#include "cAVS/ProbeService.hpp"

namespace debug_agent
{
namespace cavs
{
void ProbeService::setState(bool active)
{
    if (active) {
        try {
            std::lock_guard<std::mutex> guard(mProbeConfigMutex);
            mDriver.getProber().setProbesConfig(mProbeConfigs, getInjectionSampleByteSizes());
        } catch (Prober::Exception &e) {
            throw Exception("Cannot set probe sconfig: " + std::string(e.what()));
        }
    }
    try {
        mDriver.getProber().setState(active);
    } catch (Prober::Exception &e) {
        throw Exception("Cannot set probe service state: " + std::string(e.what()));
    }
}

/**
* Get the state of the probing service
*/
bool ProbeService::isActive()
{
    try {
        return mDriver.getProber().isActive();
    } catch (Prober::Exception &e) {
        throw Exception("Cannot get probe service state: " + std::string(e.what()));
    }
}

void ProbeService::checkProbeId(ProbeId probeId) const
{
    if (probeId.getValue() >= mDriver.getProber().getMaxProbeCount()) {
        throw Exception("Invalid probe index: " + std::to_string(probeId.getValue()));
    }
}

void ProbeService::setConfiguration(ProbeId id, const Prober::ProbeConfig &probe)
{
    checkProbeId(id);

    std::lock_guard<std::mutex> guard(mProbeConfigMutex);
    mProbeConfigs[id.getValue()] = probe;
}

Prober::ProbeConfig ProbeService::getConfiguration(ProbeId id)
{
    checkProbeId(id);

    std::lock_guard<std::mutex> guard(mProbeConfigMutex);
    return mProbeConfigs[id.getValue()];
}

const Prober::InjectionSampleByteSizes ProbeService::getInjectionSampleByteSizes() const
{
    Prober::InjectionSampleByteSizes sizeMap;
    ProbeId::RawType probeIndex = 0;

    // Iterating on enabled injection probes
    for (auto &probeConfig : mProbeConfigs) {
        if (probeConfig.enabled) {
            if (probeConfig.purpose == Prober::ProbePurpose::Inject ||
                probeConfig.purpose == Prober::ProbePurpose::InjectReextract) {

                // Getting props of the probed module instance
                dsp_fw::ModuleInstanceProps props;
                try {
                    props = mDriver.getModuleHandler().getModuleInstanceProps(
                        probeConfig.probePoint.fields.getModuleId(),
                        probeConfig.probePoint.fields.getInstanceId());
                } catch (ModuleHandler::Exception &e) {
                    throw Exception("Can not retreive injection format of probe id " +
                                    std::to_string(probeIndex) + ": " + std::string(e.what()));
                }

                // Getting the pin list
                dsp_fw::PinListInfo pinList;
                switch (probeConfig.probePoint.fields.getType()) {
                case dsp_fw::ProbeType::Input:
                    pinList = props.input_pins;
                    break;
                case dsp_fw::ProbeType::Output:
                    pinList = props.output_pins;
                    break;
                default:
                    throw Exception("Unsupported pin type: " +
                                    dsp_fw::probeTypeHelper().toString(
                                        probeConfig.probePoint.fields.getType()));
                }

                // Checking pin index validy
                if (probeConfig.probePoint.fields.getIndex() >= pinList.pin_info.size()) {
                    throw Exception("Invalid pin index: " +
                                    std::to_string(probeConfig.probePoint.fields.getIndex()) +
                                    " max: " + std::to_string(pinList.pin_info.size()));
                }

                // Checking bit depth validity
                dsp_fw::PinProps &prop = pinList.pin_info[probeConfig.probePoint.fields.getIndex()];
                if (prop.format.valid_bit_depth % 8 != 0) {
                    throw Exception("Unsupported format bit depth: " +
                                    std::to_string(prop.format.bit_depth) +
                                    " (should be a multiple of 8)");
                }

                // Calculating sample byte size and storing it in the map
                std::size_t sampleByteSize =
                    (prop.format.bit_depth / 8) * prop.format.number_of_channels;
                sizeMap[ProbeId(probeIndex)] = sampleByteSize;
            }
        }
        probeIndex++;
    }
    return sizeMap;
}
} // cavs
} // debug_agent
