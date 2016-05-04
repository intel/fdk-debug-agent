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
#include <cAVS/Linux/Logger.hpp>
#include <cAVS/Linux/DriverTypes.hpp>
#include "Util/PointerHelper.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/ByteStreamReader.hpp"
#include <Util/AssertAlways.hpp>
#include <string>
#include <cstring>
#include <algorithm>
#include <map>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

static const std::map<Logger::Level, mixer_ctl::LogPriority> levelConversion = {
    {Logger::Level::Verbose, mixer_ctl::LogPriority::Verbose},
    {Logger::Level::Low, mixer_ctl::LogPriority::Low},
    {Logger::Level::Medium, mixer_ctl::LogPriority::Medium},
    {Logger::Level::High, mixer_ctl::LogPriority::High},
    {Logger::Level::Critical, mixer_ctl::LogPriority::Critical}};

/* Explicit definition, so that it can be referenced.  See
 * https://gcc.gnu.org/wiki/VerboseDiagnostics#missing_static_const_definition */
const size_t Logger::LogProducer::fragmentSize;

void Logger::setParameters(const Parameters &parameters)
{
    std::lock_guard<std::mutex> locker(mLogActivationContextMutex);

    if (isLogProductionRunning() != parameters.mIsStarted) {
        /* Start/Stop state has changed */

        if (parameters.mIsStarted) {
            startLogLocked(parameters);
        } else {
            stopLogLocked(parameters);
        }
    } else {
        /* Start/Stop state has not changed */
        if (isLogProductionRunning()) {
            throw Exception("Can not change log parameters while logging is activated.");
        }
        updateLogLocked(parameters);
    }
}

void Logger::startLogLocked(const Parameters &parameters)
{
    ASSERT_ALWAYS(not isLogProductionRunning());

    setLogLevel(parameters.mLevel);

    /* Clearing the log queue at session start and open it during all logger session. */
    mLogEntryQueue.clear();
    mLogEntryQueue.open();

    /* Starting the producer thread after setting level of logs into the fw */
    constructProducers();
}

void Logger::stopLogLocked(const Parameters &parameters)
{
    destroyProducers();

    /* Unblocking log consumer threads */
    mLogEntryQueue.close();

    /* Update the low level anyway for consistency. */
    setLogLevel(parameters.mLevel);
}

void Logger::updateLogLocked(const Parameters &parameters)
{
    /* Cannot change log parameters during running session */
    ASSERT_ALWAYS(not isLogProductionRunning());

    setLogLevel(parameters.mLevel);
}

Logger::Parameters Logger::getParameters()
{
    /** Control Device exposes only a control to set the log level.
     * Log production is started once tinycompress devices
     * are opened and started
     */
    return {isLogProductionRunning(), getLogLevel(), Output::Sram};
}

void Logger::setLogLevel(const Level &level)
{
    util::MemoryByteStreamWriter writer;
    writer.write(toLinux(level));
    try {
        mControlDevice.ctlWrite(mixer_ctl::logLevelMixer, writer.getBuffer());
    } catch (const ControlDevice::Exception &e) {
        throw Exception("Failed to write the log level control: " + std::string(e.what()));
    }
}

Logger::Level Logger::getLogLevel() const
{
    mixer_ctl::LogPriority logPriority;

    util::Buffer logLevelBuffer{};
    try {
        mControlDevice.ctlRead(mixer_ctl::logLevelMixer, logLevelBuffer);
    } catch (const ControlDevice::Exception &e) {
        throw Exception("Failed to read the log level control: " + std::string(e.what()));
    }
    util::MemoryByteStreamReader reader(logLevelBuffer);
    reader.read(logPriority);

    return fromLinux(logPriority);
}

std::unique_ptr<LogBlock> Logger::readLogBlock()
{
    return mLogEntryQueue.remove();
}

void Logger::constructProducers()
{
    compress::LoggersInfo loggersInfo;
    try {
        loggersInfo = mCompressDeviceFactory.getLoggerDeviceInfoList();
    } catch (const CompressDeviceFactory::Exception &e) {
        throw Exception("Failed to get Logger Device Info list. " + std::string(e.what()));
    }
    for (const auto &loggerInfo : loggersInfo) {
        /* @todo: if one is failing, shall we continue in degradated mode? */
        mLogProducers.push_back(
            std::make_unique<LogProducer>(mLogEntryQueue, loggerInfo.coreId(), mDevice,
                                          mCompressDeviceFactory.newCompressDevice(loggerInfo)));
    }
    if (mLogProducers.empty()) {
        throw Exception("No Log Producers instantiated.");
    }
}

void Logger::destroyProducers()
{
    for (auto &producer : mLogProducers) {
        assert(producer != nullptr);

        /* This ensures that mLogProducer will be deleted, even if an exception is thrown */
        util::EnsureUniquePtrDeletion<LogProducer> ptrDeleter(producer);
    }
    mLogProducers.clear();
}

void Logger::stop() noexcept
{
    /* Stopping log session if one is running */
    std::lock_guard<std::mutex> locker(mLogActivationContextMutex);
    if (!mLogProducers.empty()) {

        /* Using arbitrary level and output values, they are not relevant when stopping log... */
        Parameters parameter(false, Logger::Level::Verbose, Logger::Output::Sram);
        try {
            stopLogLocked(parameter);
        } catch (const Exception &e) {
            std::cout << "Cannot stop log producer : " << e.what() << std::endl;
        }
    }
}

mixer_ctl::LogPriority Logger::toLinux(const Logger::Level &level)
{
    try {
        return levelConversion.at(level);
    } catch (std::out_of_range &) {
        throw Exception("Wrong level value (" + std::to_string(static_cast<uint32_t>(level)) +
                        ").");
    }
}

Logger::Level Logger::fromLinux(const mixer_ctl::LogPriority &level)
{
    for (auto &candidate : levelConversion) {
        if (candidate.second == level) {
            return candidate.first;
        }
    }
    throw Exception("Wrong level value (" + std::to_string(static_cast<uint32_t>(level)) + ")");
}

/* The constructor starts the log producer thread.
 * @todo: once driver supports waking up one core separately, send the request to the right core.
 */
Logger::LogProducer::LogProducer(BlockingLogQueue &queue, unsigned int coreId, Device &device,
                                 std::unique_ptr<CompressDevice> logDevice)
    : mQueue(queue), mCoreId(coreId), mLogDevice(std::move(logDevice)), mDevice(device),
      mCorePower(device)
{
    /* No parameter to start / stop logging on linux. So, just consider that if a log device
     * could be opened and started and consequently a log producer instantiated it is enough
     * to consider the log feature as started. */
    startLogDevice();

    /* Once the device is opened and started (operation that may throw and this MUST be reported),
     * launching the thread and giving exclusive ownership of the device. */
    mLogResult = std::async(std::launch::async, &Logger::LogProducer::produceEntries, this);
}

Logger::LogProducer::~LogProducer()
{
    stopLogDevice();
}

void Logger::LogProducer::startLogDevice()
{
    std::lock_guard<std::mutex> guard(mLogDeviceMutex);
    assert(mLogDevice != nullptr);

    /* First wake up associated core, or at least prevent from sleeping. */
    mCorePower.preventCoreFromSleeping();
    compress::Config config(fragmentSize, nbFragments);
    try {
        mLogDevice->open(Mode::NonBlocking, compress::Role::Capture, config);
    } catch (const CompressDevice::Exception &e) {
        mCorePower.allowCoreToSleep();
        throw Exception("Error opening Log Device: " + std::string(e.what()));
    }
    try {
        mLogDevice->start();
    } catch (const CompressDevice::Exception &e) {
        mLogDevice->close();
        mCorePower.allowCoreToSleep();
        throw Exception("Error starting Log Device: " + std::string(e.what()));
    }
}

void Logger::LogProducer::stopLogDevice()
{
    /* Using a std::unique_lock instead of a std::lock_guard because this lock
     * will be changed by the mCondVar.wait() method.
     */
    std::unique_lock<std::mutex> guard(mLogDeviceMutex);
    try {
        mLogDevice->stop();
    } catch (const CompressDevice::Exception &e) {
        /* Called from destructor, no throw... */
        std::cout << "Error stopping Log Device: " << std::string(e.what()) << std::endl;
    }
    // Ensure we can safely close the device
    if (mProductionThreadBlocked) {
        mCondVar.wait(guard);
    }
    ASSERT_ALWAYS(not mProductionThreadBlocked);

    mLogDevice->close();

    /* Can decrease core wake up ref count. */
    mCorePower.allowCoreToSleepNoExcept();
}

void Logger::LogProducer::produceEntries()
{
    assert(mLogDevice != nullptr);

    for (;;) {
        {
            std::lock_guard<std::mutex> guard(mLogDeviceMutex);
            if (not mLogDevice->isRunning()) {
                std::cout << "Log Device closed, exiting." << std::endl;
                break;
            }
            mProductionThreadBlocked = true;
        }
        try {
            if (not mLogDevice->wait(CompressDevice::mInfiniteTimeout)) {
                break;
            }
        } catch (const CompressDevice::IoException &) {
            /** Log compress device has been stopped, exiting production. */
            break;
        } catch (const CompressDevice::Exception &e) {
            std::cout << "Waiting on Log Device failed " + std::string(e.what()) << ", exiting"
                      << std::endl;
            break;
        }
        std::lock_guard<std::mutex> guard(mLogDeviceMutex);
        mProductionThreadBlocked = false;
        mCondVar.notify_one();
        /* Wait guarantees that there is something to read. */
        LogBlockPtr logBlock(std::make_unique<LogBlock>(mCoreId, fragmentSize));
        try {
            mLogDevice->read(logBlock->getLogData());
        } catch (const CompressDevice::Exception &e) {
            std::cout << "Error reading log from Log Device: " + std::string(e.what()) << std::endl;
        }
        if (logBlock->getLogData().size() != 0 && !mQueue.add(std::move(logBlock))) {
            std::cout << "Warning: dropping log entry: the queue is full or closed" << std::endl;
        }
    }
    std::lock_guard<std::mutex> guard(mLogDeviceMutex);
    mProductionThreadBlocked = false;
    mCondVar.notify_all();
}
}
}
}
