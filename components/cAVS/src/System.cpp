/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015-2016 Intel Corporation. All Rights Reserved.
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
#include "cAVS/System.hpp"
#include "cAVS/DriverFactory.hpp"
#include "cAVS/LogStreamer.hpp"
#include "Util/StringHelper.hpp"
#include "System/IfdkStreamHeader.hpp"
#include <algorithm>
#include <utility>
#include <set>
#include <map>

namespace debug_agent
{
namespace cavs
{

// System::LogStreamResource class
void System::LogStreamResource::doWriting(std::ostream &os)
{
    LogStreamer logStreamer(mLogger, mModuleEntries);
    os << logStreamer;
}

// System::ProbeStreamResource class
void System::ProbeExtractionStreamResource::doWriting(std::ostream &os)
{
    // header attributes
    const std::string systemType = "generic";
    const std::string formatType = "probe";
    const int majorVersion = 1;
    const int minorVersion = 0;

    // writing header
    system::IfdkStreamHeader header(systemType, formatType, majorVersion, minorVersion);
    os << header;

    try {
        while (true) {
            std::unique_ptr<util::Buffer> block = mProber.dequeueExtractionBlock(mProbeIndex);
            if (block == nullptr) {
                // Extraction is finished
                return;
            }
            os.write(reinterpret_cast<const char *>(block->data()), block->size());
            if (os.fail()) {
                throw Exception("Unable to write probe data to output stream");
            }
        }
    } catch (Prober::Exception &e) {
        throw Exception("Cannot extract block: " + std::string(e.what()));
    }
}

void System::ProbeInjectionStreamResource::doReading(std::istream &is)
{
    static const std::size_t bufferSize = 4096;

    util::Buffer buffer;

    try {
        while (true) {
            std::streamsize read = 0;
            buffer.resize(bufferSize);

            // Reading first byte to block until something is available
            is.read(reinterpret_cast<char *>(buffer.data()), 1);
            if (is.good()) {
                ++read;

                // Then reading other available bytes, in the limit of buffer size
                read += is.readsome(reinterpret_cast<char *>(buffer.data() + 1), buffer.size() - 1);
            }

            if (read > 0) {
                // resizing and sending the buffer
                buffer.resize(read);
                mProber.enqueueInjectionBlock(mProbeIndex, buffer);
            }

            if (is.eof()) {
                /* End of stream : returning */
                return;
            }
            if (is.fail()) {
                throw Exception("Unable to read probe data from input stream");
            }
        }
    } catch (Prober::Exception &e) {
        throw Exception("Cannot inject block: " + std::string(e.what()));
    }
}

// System class
System::System(const DriverFactory &driverFactory)
    : mDriver(std::move(createDriver(driverFactory))), mModuleEntries(), mFwConfig(), mHwConfig(),
      mProbeExtractionMutexes(mDriver->getProber().getMaxProbeCount()),
      mProbeInjectionMutexes(mDriver->getProber().getMaxProbeCount()),
      mProbeService(mDriver->getProber(), mDriver->getModuleHandler()),
      mPerfService(mDriver->getPerf(), mDriver->getModuleHandler())
{
    try {
        mDriver->getModuleHandler().getFwConfig(mFwConfig);
    } catch (ModuleHandler::Exception &e) {
        /** @todo use logging */
        std::cout << "Unable to get FW config: " + std::string(e.what()) << std::endl;
    }
    try {
        mDriver->getModuleHandler().getHwConfig(mHwConfig);
    } catch (ModuleHandler::Exception &e) {
        /** @todo use logging */
        std::cout << "Unable to get HW config: " + std::string(e.what()) << std::endl;
    }

    if (mFwConfig.isModulesCountValid) {
        try {
            mDriver->getModuleHandler().getModulesEntries(mFwConfig.modulesCount, mModuleEntries);
        } catch (ModuleHandler::Exception &e) {
            /** @todo use logging */
            std::cout << "Unable to get module entries: " + std::string(e.what()) << std::endl;
        }
    } else {
        /** @todo use logging */
        std::cout << "Cannot get module entries: module count is invalid." << std::endl;
    }
    if (mFwConfig.isMaxModInstCountValid and mHwConfig.isDspCoreCountValid) {
        mPerfService.setMaxItemCount(mFwConfig.maxModInstCount + mHwConfig.dspCoreCount);
    } else {
        std::cout << "Perf Service won't work: can't retrieve the maximum amount of perf items."
                  << std::endl;
    }
}

std::unique_ptr<Driver> System::createDriver(const DriverFactory &driverFactory)
{
    try {
        auto driver = driverFactory.newDriver();
        if (driver == nullptr) {
            throw Exception("Driver factory has failed");
        }
        return driver;
    } catch (DriverFactory::Exception e) {
        throw Exception("Unable to create driver: " + std::string(e.what()));
    }
}

void System::setLogParameters(Logger::Parameters &parameters)
{
    try {
        mDriver->getLogger().setParameters(parameters);
    } catch (Logger::Exception &e) {
        throw Exception("Unable to set log parameter: " + std::string(e.what()));
    }
}

Logger::Parameters System::getLogParameters()
{
    try {
        return mDriver->getLogger().getParameters();
    } catch (Logger::Exception &e) {
        throw Exception("Unable to get log parameter: " + std::string(e.what()));
    }
}

const std::vector<dsp_fw::ModuleEntry> &System::getModuleEntries() const noexcept
{
    return mModuleEntries;
}

const dsp_fw::ModuleEntry &System::findModuleEntry(uint16_t moduleId) const
{
    for (auto &module : mModuleEntries) {
        if (module.module_id == moduleId)
            return module;
    }
    throw Exception("module with id '" + std::to_string(moduleId) + "' not found");
}

const dsp_fw::ModuleEntry &System::findModuleEntry(const std::string &name) const
{
    for (auto &module : mModuleEntries) {
        if (module.getName() == name)
            return module;
    }
    throw Exception("module with name  '" + name + "' not found");
}

const dsp_fw::FwConfig &System::getFwConfig() const noexcept
{
    return mFwConfig;
}

const dsp_fw::HwConfig &System::getHwConfig() const noexcept
{
    return mHwConfig;
}

template <typename T>
std::unique_ptr<T> System::tryToAcquireResource(std::unique_ptr<T> resource)
{
    if (resource->tryLock()) {
        return resource;
    } else {
        return nullptr;
    }
}

std::unique_ptr<System::OutputStreamResource> System::tryToAcquireLogStreamResource()
{
    return tryToAcquireResource(std::make_unique<System::LogStreamResource>(
        mLogStreamMutex, mDriver->getLogger(), mModuleEntries));
}

void System::checkProbeIndex(ProbeId probeIndex) const
{
    if (probeIndex.getValue() >= mDriver->getProber().getMaxProbeCount()) {
        throw Exception("Wrong probe index: " + std::to_string(probeIndex.getValue()));
    }
}

std::unique_ptr<System::OutputStreamResource> System::tryToAcquireProbeExtractionStreamResource(
    ProbeId probeIndex)
{
    checkProbeIndex(probeIndex);

    return tryToAcquireResource(std::make_unique<System::ProbeExtractionStreamResource>(
        mProbeExtractionMutexes[probeIndex.getValue()], mDriver->getProber(), probeIndex));
}

std::unique_ptr<System::InputStreamResource> System::tryToAcquireProbeInjectionStreamResource(
    ProbeId probeIndex)
{
    checkProbeIndex(probeIndex);

    return tryToAcquireResource(std::make_unique<System::ProbeInjectionStreamResource>(
        mProbeInjectionMutexes[probeIndex.getValue()], mDriver->getProber(), probeIndex));
}

void System::setModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                dsp_fw::ParameterId parameterId,
                                const util::Buffer &parameterPayload)
{
    mDriver->getModuleHandler().setModuleParameter(moduleId, instanceId, parameterId,
                                                   parameterPayload);
}

void System::getModuleParameter(uint16_t moduleId, uint16_t instanceId,
                                dsp_fw::ParameterId parameterId, util::Buffer &parameterPayload)
{
    mDriver->getModuleHandler().getModuleParameter(moduleId, instanceId, parameterId,
                                                   parameterPayload);
}

void System::getTopology(Topology &topology)
{
    topology.clear();

    ModuleHandler &handler = mDriver->getModuleHandler();
    std::set<dsp_fw::CompoundModuleId> moduleInstanceIds;

    /* Retrieving gateways*/
    if (!mHwConfig.isGatewayCountValid) {
        throw Exception("Gate count is invalid.");
    }

    try {
        handler.getGatewaysInfo(mHwConfig.gatewayCount, topology.gateways);
    } catch (ModuleHandler::Exception &e) {
        throw Exception("Can not retrieve gateways: " + std::string(e.what()));
    }

    /* Retrieving pipelines ids*/
    if (!mFwConfig.isMaxPplCountValid) {
        throw Exception("Max pipeline count is invalid.");
    }

    std::vector<dsp_fw::PipeLineIdType> pipelineIds;
    try {
        handler.getPipelineIdList(mFwConfig.maxPplCount, pipelineIds);
    } catch (ModuleHandler::Exception &e) {
        throw Exception("Can not retrieve pipeline ids: " + std::string(e.what()));
    }

    /* Retrieving pipeline props*/
    for (auto pplId : pipelineIds) {
        try {
            dsp_fw::PplProps props;
            handler.getPipelineProps(pplId, props);
            topology.pipelines.push_back(props);

            /* Collecting module instance ids*/
            for (auto instanceId : props.module_instances) {
                moduleInstanceIds.insert(instanceId);
            }
        } catch (ModuleHandler::Exception &e) {
            throw Exception("Can not retrieve pipeline props of id " +
                            std::to_string(pplId.getValue()) + " : " + std::string(e.what()));
        }
    }
    /* According to the SwAS the pipe collection has to be ordered from
     * highest priority to lowest priority (highest priority is lowest value) */
    std::sort(topology.pipelines.begin(), topology.pipelines.end(),
              [](dsp_fw::PplProps pipeA, dsp_fw::PplProps pipeB) {
                  return pipeA.priority < pipeB.priority;
              });

    /* Retrieving scheduler props*/
    if (!mHwConfig.isDspCoreCountValid) {
        throw Exception("Core count is invalid.");
    }

    for (uint32_t coreId = 0; coreId < mHwConfig.dspCoreCount; coreId++) {
        try {
            dsp_fw::SchedulersInfo info;
            handler.getSchedulersInfo(dsp_fw::CoreId{coreId}, info);
            topology.schedulers.push_back(info);

            /* Collecting module instance ids*/
            for (auto &scheduler : info.scheduler_info) {
                for (auto &task : scheduler.task_info) {
                    for (auto &instanceId : task.module_instance_id) {
                        moduleInstanceIds.insert(instanceId);
                    }
                }
            }
        } catch (ModuleHandler::Exception &e) {
            throw Exception("Can not retrieve scheduler props of core id: " +
                            std::to_string(coreId) + " : " + std::string(e.what()));
        }
    }

    /* Retrieving module instances*/
    for (auto &compoundId : moduleInstanceIds) {
        try {
            dsp_fw::ModuleInstanceProps props;
            handler.getModuleInstanceProps(compoundId.moduleId, compoundId.instanceId, props);
            topology.moduleInstances[props.id] = props;
        } catch (ModuleHandler::Exception &e) {
            throw Exception("Can not retrieve module instance with id: (" +
                            std::to_string(compoundId.moduleId) + "," +
                            std::to_string(compoundId.instanceId) + ") : " + std::string(e.what()));
        }
    }

    /* Compute links */
    topology.computeLinks();
}

void System::setProberState(bool active)
{
    try {
        mProbeService.setState(active);
    } catch (ProbeService::Exception &e) {
        throw Exception("Cannot set probe service state: " + std::string(e.what()));
    }
}

/**
* Get the state of the probing service
*/
bool System::isProberActive()
{
    try {
        return mProbeService.isActive();
    } catch (ProbeService::Exception &e) {
        throw Exception("Cannot get probe service state: " + std::string(e.what()));
    }
}

void System::setProbeConfiguration(ProbeId id, const Prober::ProbeConfig &probe)
{
    try {
        return mProbeService.setProbeConfig(id, probe);
    } catch (ProbeService::Exception &e) {
        throw Exception("Cannot set probe sconfig: " + std::string(e.what()));
    }
}

Prober::ProbeConfig System::getProbeConfiguration(ProbeId id)
{
    try {
        return mProbeService.getProbeConfig(id);
    } catch (ProbeService::Exception &e) {
        throw Exception("Cannot get probe config: " + std::string(e.what()));
    }
}

Perf::State System::getPerfState()
{
    return mPerfService.getState();
}

void System::setPerfState(Perf::State state)
{
    mPerfService.setState(state);
}

PerfService::CompoundPerfData System::getPerfData()
{
    return mPerfService.getData();
}
}
}
