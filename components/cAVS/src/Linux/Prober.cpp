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
      mIsActiveState(false)
{
    try {
        mMaxExtractionProbes = mControlDevice.getControlCountByTag(mixer_ctl::mExtractorControlTag);
        mMaxInjectionProbes = mControlDevice.getControlCountByTag(mixer_ctl::mInjectorControlTag);

    } catch (const ControlDevice::Exception &e) {
        throw Exception("Failed to retrieve max probe count: " + std::string(e.what()));
    }

    mCachedProbeConfig = SessionProbes(getMaxProbeCount());
    for (std::size_t injectIndex = 0; injectIndex < mMaxInjectionProbes; ++injectIndex) {
        mInjectionQueues.emplace_back(mQueueSize);
    }
    for (std::size_t extractIndex = 0; extractIndex < mMaxExtractionProbes; ++extractIndex) {
        mExtractionQueues.emplace_back(mQueueSize,
                                       [](const util::Buffer &buffer) { return buffer.size(); });
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

void Prober::setProbesConfig(const SessionProbes &probes,
                             const InjectionSampleByteSizes &injectionSampleByteSizes)
{
    std::lock_guard<std::mutex> guard(mProbeConfigMutex);

    /** Sanity check on probes configuration. */
    for (const auto &probe : probes) {
        toLinux(probe);
    }
    mCachedProbeConfig = probes;
    mCachedInjectionSampleByteSizes = injectionSampleByteSizes;
    mExtractionProbeMap.clear();
    mInjectionProbeMap.clear();

    /* NOTE: there is only one compress device for extraction but as many control as extraction
     * end points. So, we must refresh ALL the mixer control before starting an extraction probe
     * session, disabled extraction point will be set with default config.
     */
    SessionProbes extractionProbeConfiguration{
        mMaxExtractionProbes, {false, {0, 0, dsp_fw::ProbeType::Output, 0}, ProbePurpose::Extract}};

    auto result = getActiveSession(mCachedProbeConfig);
    auto &extractionProbes = result.first;
    auto &injectionProbes = result.second;

    if (extractionProbes.size() > mMaxExtractionProbes) {
        throw Exception("Exceed max extraction probes supported: " + extractionProbes.size());
    }
    if (injectionProbes.size() > mMaxInjectionProbes) {
        throw Exception("Exceed max injection probes supported: " + injectionProbes.size());
    }

    /** Setting extraction probe mixers. */
    if (not extractionProbes.empty()) {
        ProbeId::RawType probeControlId{0};
        for (auto probeId : extractionProbes) {
            extractionProbeConfiguration[probeControlId] = mCachedProbeConfig[probeId.getValue()];
            mExtractionProbeMap[probeId] = ProbeId{probeControlId};
            probeControlId++;
        }
    }
    ProbeId::RawType probeControlId{0};
    for (const auto &extractionProbe : extractionProbeConfiguration) {
        mixer_ctl::ProbeControl probeControl(toLinux(extractionProbe));
        util::MemoryByteStreamWriter controlWriter;
        controlWriter.write(probeControl);
        try {
            mControlDevice.ctlWrite(mixer_ctl::getProbeExtractControl(probeControlId),
                                    controlWriter.getBuffer());
        } catch (const ControlDevice::Exception &e) {
            throw Exception("Failed to write extration probe control settings: " +
                            std::string(e.what()));
        }
        probeControlId++;
    }

    /** Setting injection probe mixers. */
    if (not injectionProbes.empty()) {
        ProbeId::RawType probeControlId{0};
        for (auto probeId : injectionProbes) {
            const auto &probe = mCachedProbeConfig[probeId.getValue()];
            mixer_ctl::ProbeControl probeControl(toLinux(probe));
            util::MemoryByteStreamWriter controlWriter;
            controlWriter.write(probeControl);
            try {
                mControlDevice.ctlWrite(mixer_ctl::getProbeInjectControl(probeControlId),
                                        controlWriter.getBuffer());
                mInjectionProbeMap[probeId] = ProbeId{probeControlId};
                probeControlId++;
            } catch (const ControlDevice::Exception &e) {
                throw Exception("Failed to write injection probe control settings:: " +
                                std::string(e.what()));
            }
        }
    }
}

Prober::SessionProbes Prober::getProbesConfig() const
{
    std::lock_guard<std::mutex> guard(mProbeConfigMutex);
    // Control mixer for probe configuration is write only for injection, returns null values
    // for extraction.
    return mCachedProbeConfig;
}

ProbeId Prober::getInjectProbeControlId(const ProbeId &probeIndex) const
{
    ProbeId probeControlId;
    try {
        probeControlId = mInjectionProbeMap.at(probeIndex);
    } catch (std::out_of_range &) {
        throw Exception("Unknown probe point id: " + std::to_string(probeIndex.getValue()));
    }
    // Checking that the probe index is in a valid range
    if (probeControlId.getValue() >= mMaxInjectionProbes) {
        throw Exception("Wrong probe id: " + std::to_string(probeIndex.getValue()));
    }
    return probeControlId;
}

ProbeId Prober::getExtractProbeControlId(const ProbeId &probeIndex) const
{
    ProbeId probeControlId;
    try {
        probeControlId = mExtractionProbeMap.at(probeIndex);
    } catch (std::out_of_range &) {
        throw Exception("Unknown probe point id: " + std::to_string(probeIndex.getValue()));
    }
    // Checking that the probe index is in a valid range
    if (probeControlId.getValue() >= mMaxExtractionProbes) {
        throw Exception("Wrong probe id: " + std::to_string(probeIndex.getValue()));
    }
    return probeControlId;
}

std::unique_ptr<util::Buffer> Prober::dequeueExtractionBlock(ProbeId probeIndex)
{
    ProbeId probeControlId{getExtractProbeControlId(probeIndex)};

    return mExtractionQueues[probeControlId.getValue()].remove();
}

bool Prober::enqueueInjectionBlock(ProbeId probeIndex, const util::Buffer &buffer)
{
    ProbeId probeControlId{getInjectProbeControlId(probeIndex)};

    // Blocks if the injection queue is full
    return mInjectionQueues[probeControlId.getValue()].writeBlocking(buffer.data(), buffer.size());
}

void Prober::startStreaming()
{
    auto result = getActiveSession(mCachedProbeConfig);
    auto &extractionProbes = result.first;
    auto &injectionProbes = result.second;

    if (not extractionProbes.empty()) {

        // opening queues of the active probes only
        ProbeExtractor::ProbePointMap probePointMap;
        for (auto probeId : extractionProbes) {
            ProbeId probeControlId{getExtractProbeControlId(probeId)};
            mExtractionQueues[probeControlId.getValue()].open();

            dsp_fw::ProbePointId probePointId{mCachedProbeConfig[probeId.getValue()].probePoint};
            // Checking that the probe point id is not already in the map
            if (probePointMap.find(probePointId) != probePointMap.end()) {
                throw Exception("Two active extraction probes have the same probe point id: " +
                                probePointId.toString());
            }
            probePointMap[probePointId] = probeControlId;
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
        ProbeId probeControlId{getInjectProbeControlId(probeId)};
        mInjectionQueues[probeControlId.getValue()].open();

        // Finding associated sample byte size
        auto it = mCachedInjectionSampleByteSizes.find(probeId);
        if (it == mCachedInjectionSampleByteSizes.end()) {
            throw Exception("Sample byte size not found for injection probe id " +
                            std::to_string(probeId.getValue()));
        }
        auto sampleByteSize = it->second;
        if (sampleByteSize == 0) {
            throw Exception("Sample byte size must be greater than 0 for injection probe id " +
                            std::to_string(probeId.getValue()));
        }

        compress::InjectionProbesInfo injectionProbesInfo;
        try {
            injectionProbesInfo = mCompressDeviceFactory.getInjectionProbeDeviceInfoList();
        } catch (const CompressDeviceFactory::Exception &e) {
            throw Exception("Failed to get Extract Probe Devices Info. " + std::string(e.what()));
        }
        /* @todo: ensure that the number of injection device matches the maxInjectionProbe. */
        std::unique_ptr<InjectionOutputStream> outputStream;
        try {
            outputStream =
                std::make_unique<InjectionOutputStream>(mCompressDeviceFactory.newCompressDevice(
                    injectionProbesInfo[probeControlId.getValue()]));
        } catch (const InjectionOutputStream::Exception &e) {
            throw Exception("Could not start extraction output stream: " + std::string(e.what()));
        }

        // Creating injector
        mProbeInjectors.emplace_back(std::move(outputStream),
                                     mInjectionQueues[probeControlId.getValue()], sampleByteSize);
    }
}

void Prober::stopStreaming()
{
    // Deleting extractor (the packet producer)
    mProbeExtractor.reset();

    // deleting injectors
    mProbeInjectors.clear();

    // then closing the queues to make wakup listening threads
    for (auto probe : mExtractionProbeMap) {
        mExtractionQueues[probe.second.getValue()].close();
    }
    for (auto probe : mInjectionProbeMap) {
        mInjectionQueues[probe.second.getValue()].close();
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
