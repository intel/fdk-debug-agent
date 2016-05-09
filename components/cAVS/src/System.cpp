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
    : mDriver(std::move(createDriver(driverFactory))), mProbeService(*mDriver),
      mProbeExtractionMutexes(mDriver->getProber().getMaxProbeCount()),
      mProbeInjectionMutexes(mDriver->getProber().getMaxProbeCount()),
      mPerfService(mDriver->getPerf(), getModuleHandler())
{
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

std::unique_ptr<System::OutputStreamResource> System::tryToAcquireProbeExtractionStreamResource(
    ProbeId probeIndex)
{
    mProbeService.checkProbeId(probeIndex);

    return tryToAcquireResource(std::make_unique<System::ProbeExtractionStreamResource>(
        mProbeExtractionMutexes[probeIndex.getValue()], mDriver->getProber(), probeIndex));
}

std::unique_ptr<System::InputStreamResource> System::tryToAcquireProbeInjectionStreamResource(
    ProbeId probeIndex)
{
    mProbeService.checkProbeId(probeIndex);

    return tryToAcquireResource(std::make_unique<System::ProbeInjectionStreamResource>(
        mProbeInjectionMutexes[probeIndex.getValue()], mDriver->getProber(), probeIndex));
}

void System::getTopology(Topology &topology)
{
    topology.clear();

    ModuleHandler &handler = getModuleHandler();
    std::set<dsp_fw::CompoundModuleId> moduleInstanceIds;

    auto &hwConfig = handler.getHwConfig();

    try {
        topology.gateways = handler.getGatewaysInfo();
    } catch (ModuleHandler::Exception &e) {
        throw Exception("Can not retrieve gateways: " + std::string(e.what()));
    }

    std::vector<dsp_fw::PipeLineIdType> pipelineIds;
    try {
        pipelineIds = handler.getPipelineIdList();
    } catch (ModuleHandler::Exception &e) {
        throw Exception("Can not retrieve pipeline ids: " + std::string(e.what()));
    }

    /* Retrieving pipeline props*/
    for (auto pplId : pipelineIds) {
        try {
            dsp_fw::PplProps props = handler.getPipelineProps(pplId);
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

    for (uint32_t coreId = 0; coreId < hwConfig.dspCoreCount; coreId++) {
        try {
            dsp_fw::SchedulersInfo info = handler.getSchedulersInfo(dsp_fw::CoreId{coreId});
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
            topology.moduleInstances[compoundId] =
                handler.getModuleInstanceProps(compoundId.moduleId, compoundId.instanceId);
        } catch (ModuleHandler::Exception &e) {
            throw Exception("Can not retrieve module instance with id: (" +
                            std::to_string(compoundId.moduleId) + "," +
                            std::to_string(compoundId.instanceId) + ") : " + std::string(e.what()));
        }
    }

    /* Compute links */
    topology.computeLinks();
}

ModuleHandler &System::getModuleHandler()
{
    return mDriver->getModuleHandler();
}

ProbeService &System::getProbeService()
{
    return mProbeService;
}

PerfService &System::getPerfService()
{
    return mPerfService;
}
}
}
