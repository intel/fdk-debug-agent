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
