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

#include "cAVS/Linux/Prober.hpp"
#include "cAVS/Linux/Probe/ExtractionInputStream.hpp"
#include "cAVS/Linux/Probe/InjectionOutputStream.hpp"
#include "cAVS/Linux/ControlDeviceTypes.hpp"

#include "Util/AssertAlways.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"

#include <functional>
#include <string>
#include <cstring>
#include <map>
#include <algorithm>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

/* Explicit definition, so that it can be referenced.  See
 * https://gcc.gnu.org/wiki/VerboseDiagnostics#missing_static_const_definition */
const size_t Prober::mQueueSize;

static const std::map<Prober::ProbePurpose, mixer_ctl::ProbePurpose> purposeConversion = {
    {Prober::ProbePurpose::Inject, mixer_ctl::ProbePurpose::Inject},
    {Prober::ProbePurpose::Extract, mixer_ctl::ProbePurpose::Extract},
    {Prober::ProbePurpose::InjectReextract, mixer_ctl::ProbePurpose::InjectReextract}};

Prober::Prober(ControlDevice &controlDevice, CompressDeviceFactory &compressDeviceFactory)
    : mControlDevice(controlDevice), mCompressDeviceFactory(compressDeviceFactory),
      mCachedProbeConfiguration(getMaxProbeCount()), mIsActiveState(false)
{
    for (std::size_t probeIndex = 0; probeIndex < getMaxProbeCount(); ++probeIndex) {
        mExtractionQueues.emplace_back(mQueueSize,
                                       [](const util::Buffer &buffer) { return buffer.size(); });
        mInjectionQueues.emplace_back(mQueueSize);
    }
}

Prober::~Prober()
{
    stopNoThrow();
}

void Prober::setState(bool active)
{
    std::lock_guard<std::mutex> guard(mServiceStateMutex);
    if (mIsActiveState == active) {
        return;
    }
    if (active) {
        try {
            startStreaming();
        } catch (Exception &) {
            /* Trying to stop if starting has failed to come back to a consistent state */
            stopNoThrow();
            throw;
        }
    } else {
        stopStreaming();
    }
    mIsActiveState = active;
}

bool Prober::isActive()
{
    std::lock_guard<std::mutex> guard(mServiceStateMutex);
    return mIsActiveState;
}

void Prober::stopNoThrow() noexcept
{
    try {
        stopStreaming();
    } catch (Exception &e) {
        /* @todo : use log */
        std::cout << "Unable to stop probe service: " << e.what() << "\n";
    }
}

void Prober::checkProbeId(ProbeId probeId) const
{
    if (probeId.getValue() >= getMaxProbeCount()) {
        throw Exception("Invalid probe index: " + std::to_string(probeId.getValue()));
    }
}

void Prober::setProbesConfig(const SessionProbes &probes,
                             const InjectionSampleByteSizes &injectionSampleByteSizes)
{
    std::lock_guard<std::mutex> guard(mProbeConfigMutex);

    mCachedProbeConfiguration = probes;
    mCachedInjectionSampleByteSizes = injectionSampleByteSizes;

    ProbeId::RawType probeId = 0;
    for (auto &probe : probes) {
        mixer_ctl::ProbeControl probeControl(toLinux(probe));
        util::MemoryByteStreamWriter controlWriter;
        controlWriter.write(probeControl);
        try {
            if (probe.purpose == ProbePurpose::Inject ||
                probe.purpose == ProbePurpose::InjectReextract) {
                mControlDevice.ctlWrite(mixer_ctl::getProbeInjectControlMixer(probeId),
                                        controlWriter.getBuffer());
            }
            if (probe.purpose == ProbePurpose::Extract ||
                probe.purpose == ProbePurpose::InjectReextract) {
                mControlDevice.ctlWrite(mixer_ctl::getProbeExtractControlMixer(probeId),
                                        controlWriter.getBuffer());
            }
        } catch (const ControlDevice::Exception &e) {
            throw Exception("Control Mixer failed: " + std::string(e.what()));
        }
        probeId++;
    }
}

Prober::SessionProbes Prober::getProbesConfig() const
{
    std::lock_guard<std::mutex> guard(mProbeConfigMutex);

    // Control mixer for probe configuration is write only for injection, returns null values
    // for extraction.
    return mCachedProbeConfiguration;
}

std::unique_ptr<util::Buffer> Prober::dequeueExtractionBlock(ProbeId probeIndex)
{
    checkProbeId(probeIndex);
    return mExtractionQueues[probeIndex.getValue()].remove();
}

bool Prober::enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer)
{
    checkProbeId(probeIndex);

    // Blocks if the injection queue is full
    return mInjectionQueues[probeIndex.getValue()].writeBlocking(buffer.data(), buffer.size());
}

void Prober::startStreaming()
{
    auto result = getActiveSession(mCachedProbeConfiguration);
    auto &extractionProbes = result.first;
    auto &injectionProbes = result.second;

    if (not extractionProbes.empty()) {

        // opening queues of the active probes only
        for (auto probeId : extractionProbes) {
            mExtractionQueues[probeId.getValue()].open();
        }
        ProbeExtractor::ProbePointMap probePointMap;
        for (auto probeId : extractionProbes) {
            mExtractionQueues[probeId.getValue()].open();

            dsp_fw::ProbePointId probePointId =
                mCachedProbeConfiguration[probeId.getValue()].probePoint;
            // Checking that the probe point id is not already in the map
            if (probePointMap.find(probePointId) != probePointMap.end()) {
                throw Exception("Two active extraction probes have the same probe point id: " +
                                probePointId.toString());
            }

            probePointMap[probePointId] = probeId;
        }

        compress::ExtractionProbeInfo extractProbeInfo{0, 0};
        try {
            extractProbeInfo = mCompressDeviceFactory.getExtractionProbeDeviceInfo();
        } catch (const CompressDeviceFactory::Exception &e) {
            throw Exception("Failed to get Extract Probe Devices Info. " + std::string(e.what()));
        }
        /** Probe points are multiplexed by driver in extraction so the driver exposes one device.*/
        std::unique_ptr<ExtractionInputStream> inputStream;
        try {
            inputStream = std::make_unique<ExtractionInputStream>(
                mCompressDeviceFactory.newCompressDevice(extractProbeInfo));
        } catch (const ExtractionInputStream::Exception &e) {
            throw Exception("Could not start extraction input stream: " + std::string(e.what()));
        }
        try {
            mProbeExtractor = std::make_unique<ProbeExtractor>(mExtractionQueues, probePointMap,
                                                               std::move(inputStream));
        } catch (const ProbeExtractor::Exception &e) {
            throw Exception("Could not start extraction input stream: " + std::string(e.what()));
        }
    }
    // opening queues of the active probes and creating injectors
    for (auto probeId : injectionProbes) {
        mInjectionQueues[probeId.getValue()].open();

        // Finding associated sample byte size
        auto it = mCachedInjectionSampleByteSizes.find(probeId);
        if (it == mCachedInjectionSampleByteSizes.end()) {
            throw Exception("Sample byte size not found for injection probe id " +
                            std::to_string(probeId.getValue()));
        }
        if (it->second == 0) {
            throw Exception("Sample byte size must be greater than 0 for injection probe id " +
                            std::to_string(probeId.getValue()));
        }

        compress::InjectionProbesInfo injectionProbesInfo;
        try {
            injectionProbesInfo = mCompressDeviceFactory.getInjectionProbeDeviceInfoList();
        } catch (const CompressDeviceFactory::Exception &e) {
            throw Exception("Failed to get Extract Probe Devices Info. " + std::string(e.what()));
        }
        std::unique_ptr<InjectionOutputStream> outputStream;
        try {
            outputStream = std::make_unique<InjectionOutputStream>(
                mCompressDeviceFactory.newCompressDevice(injectionProbesInfo[0]));
        } catch (const InjectionOutputStream::Exception &e) {
            throw Exception("Could not start extraction output stream: " + std::string(e.what()));
        }

        // Creating injector
        mProbeInjectors.emplace_back(std::move(outputStream), mInjectionQueues[probeId.getValue()],
                                     it->second);
    }
}

void Prober::stopStreaming()
{
    if (mProbeExtractor == nullptr) {
        // Nothing to do
        return;
    }
    // Deleting extractor (the packet producer)
    mProbeExtractor.reset();

    // deleting injectors
    mProbeInjectors.clear();

    // then closing the queues to make wakup listening threads
    for (std::size_t probeIndex = 0; probeIndex < getMaxProbeCount(); ++probeIndex) {
        mExtractionQueues[probeIndex].close();
    }
}

mixer_ctl::ProbeState Prober::toLinux(bool connected)
{
    return connected ? mixer_ctl::ProbeState::Connect : mixer_ctl::ProbeState::Disconnect;
}

bool Prober::fromLinux(const mixer_ctl::ProbeState &state)
{
    switch (state) {
    case mixer_ctl::ProbeState::Connect:
        return true;
    case mixer_ctl::ProbeState::Disconnect:
        return false;
    }
    throw Exception("Unknown state value.");
}

mixer_ctl::ProbePurpose Prober::toLinux(const ProbePurpose &purpose)
{
    try {
        return purposeConversion.at(purpose);
    } catch (std::out_of_range &) {
        throw Exception("Wrong purpose value (" + std::to_string(static_cast<uint32_t>(purpose)) +
                        ").");
    }
}

Prober::ProbePurpose Prober::fromLinux(const mixer_ctl::ProbePurpose &purpose)
{
    for (auto &candidate : purposeConversion) {
        if (candidate.second == purpose) {
            return candidate.first;
        }
    }
    throw Exception("Wrong purpose value (" + std::to_string(static_cast<uint32_t>(purpose)) + ")");
}

Prober::ProbeConfig Prober::fromLinux(const mixer_ctl::ProbeControl &control)
{
    return {fromLinux(control.getState()), control.getPointId(), fromLinux(control.getPurpose())};
}

mixer_ctl::ProbeControl Prober::toLinux(const ProbeConfig &config)
{
    return {toLinux(config.enabled), toLinux(config.purpose), config.probePoint};
}
}
}
}
