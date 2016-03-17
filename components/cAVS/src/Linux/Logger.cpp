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
#include "Util/PointerHelper.hpp"
#include "Util/ByteStreamWriter.hpp"
#include "Util/ByteStreamReader.hpp"
#include <string>
#include <cstring>
#include <algorithm>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

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
        } else {
            updateLogLocked(parameters);
        }
    }
}

void Logger::startLogLocked(const Parameters &parameters)
{
    assert(!isLogProductionRunning());

    /* Log level must be set before starting the compress devices for logging as per HLD. */
    setLogParameterCtl(parameters.mLevel);

    /* Starting the producer thread after setting level of logs into the fw */
    constructProducers();
}

void Logger::stopLogLocked(const Parameters &parameters)
{
    destroyProducers();

    /* Update anyway parameters? */
    setLogParameterCtl(parameters.mLevel);
}

void Logger::updateLogLocked(const Parameters &parameters)
{
    /* Cannot change log parameters during running session */
    assert(!isLogProductionRunning());

    /** Control Device exposes only a control to set the log level */
    setLogParameterCtl(parameters.mLevel);
}

void Logger::setLogParameterCtl(const Level &level)
{
    util::MemoryByteStreamWriter writer;
    writer.write(static_cast<long>(translateToMixer(level)));
    try {
        mControlDevice.ctlWrite(mixer_ctl::logLevelMixer, writer.getBuffer());
    } catch (const ControlDevice::Exception &e) {
        throw Exception("Failed to write the log level control: " + std::string(e.what()));
    }
}

Logger::Level Logger::getLogParameterCtl() const
{
    mixer_ctl::LogPriority logPriority;

    util::Buffer logLevelBuffer{};
    try {
        mControlDevice.ctlRead(mixer_ctl::logLevelMixer, logLevelBuffer);
    } catch (const ControlDevice::Exception &e) {
        throw Exception("Failed to read the log level control: " + std::string(e.what()));
    }
    util::MemoryByteStreamReader reader(logLevelBuffer);
    reader.read(reinterpret_cast<long &>(logPriority));

    return translateFromMixer(logPriority);
}

Logger::Parameters Logger::getParameters()
{
    /** Control Device exposes only a control to set the log level.
     * Log production is started once tinycompress devices
     * are opened and started
     */
    return {isLogProductionRunning(), getLogParameterCtl(), Output::Sram};
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

void Logger::resetProducers()
{
    for (auto &producer : mLogProducers) {
        assert(producer != nullptr);
        /* Stopping the log producer thread in exception case */
        producer.reset();
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
    /* Unblocking log consumer threads */
    mLogEntryQueue.close();
}

void Logger::LogProducer::sendCommand(CommandPtr cmd)
{
    if (!mCommandQueue.add(std::move(cmd))) {
        throw Exception("Cannot send command: the queue is full or closed.");
    }
}

/* The constructor starts the log producer thread */
Logger::LogProducer::LogProducer(BlockingLogQueue &queue, unsigned int coreId, Device &device,
                                 std::unique_ptr<CompressDevice> logDevice)
    : mQueue(queue), mCoreId(coreId), mLogDevice(std::move(logDevice)), mDevice(device),
      mCommandQueue(maxCommandQueueSize, commandSize)
{
    /* No parameter to start / stop logging on linux. So, just consider that if a log device
     * could be opened and started and consequently a log producer instantiated it is enough
     * to consider the log feature as started. */
    startLogDevice();

    /* Open the command queue before launching the producer thread in order not to loose command
     * that could be sent before the thread has been started. */
    mCommandQueue.open();

    /* Once the device is opened and started (operation that may throw and this MUST be reported),
     * launching the thread and giving exclusive ownership of the device. */
    mProducerThread = std::thread(&Logger::LogProducer::produceEntries, this);
}

Logger::LogProducer::~LogProducer()
{
    stopProducerThread();
    mProducerThread.join();

    /* Once the thread has joined, safely take back ownership of the device to stop it. */
    stopLogDevice();
}

void Logger::LogProducer::startLogDevice()
{
    assert(mLogDevice != nullptr);

    /* First wake up associated core, or at least prevent from sleeping. */
    try {
        mDevice.setCorePowerState(mCoreId, false);
    } catch (const Device::Exception &e) {
        throw Exception("Error: could not set core power: " + std::string(e.what()));
    }
    compress::Config config(fragmentSize, nbFragments);
    try {
        mLogDevice->open(Mode::NonBlocking, Role::Capture, config);
    } catch (const CompressDevice::Exception &e) {
        setCoreAllowedToSleep();
        throw Exception("Error opening Log Device: " + std::string(e.what()));
    }
    try {
        mLogDevice->start();
    } catch (const CompressDevice::Exception &e) {
        mLogDevice->close();
        setCoreAllowedToSleep();
        throw Exception("Error starting Log Device: " + std::string(e.what()));
    }
}

void Logger::LogProducer::stopLogDevice()
{
    try {
        mLogDevice->stop();
    } catch (const CompressDevice::Exception &e) {
        /* Called from destructor, no throw... */
        std::cout << "Error stopping Log Device: " << std::string(e.what()) << std::endl;
    }
    mLogDevice->close();

    /* Can decrease core wake up ref count. */
    setCoreAllowedToSleep();
}

void Logger::LogProducer::setCoreAllowedToSleep() noexcept
{
    try {
        mDevice.setCorePowerState(mCoreId, true);
    } catch (const Device::Exception &e) {
        std::cout << "Error: could not restore core power:" << std::string(e.what()) << std::endl;
    }
}

void Logger::LogProducer::produceEntries()
{
    assert(mLogDevice != nullptr);

    for (;;) {
        /* Check command queue first. */
        if (mCommandQueue.getElementCount()) {
            CommandPtr cmd = mCommandQueue.remove();
            if (cmd != nullptr && *cmd == Command::Exit) {
                break;
            }
            std::cout << "Error: Unrecognized message posted in command queue" << std::endl;
        }
        try {
            if (!mLogDevice->wait(maxPollWaitMs)) {
                /* Timeout expire, underrun occurs, retry wait, may the timeout be increased... */
                continue;
            }
        } catch (const CompressDevice::Exception &e) {
            std::cout << "Error: waiting Log Device failed " + std::string(e.what()) << std::endl;
        }
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
    mCommandQueue.close();
}

mixer_ctl::LogPriority Logger::translateToMixer(Level level)
{
    switch (level) {
    case Level::Verbose:
        return mixer_ctl::LogPriority::Verbose;
    case Level::Low:
        return mixer_ctl::LogPriority::Low;
    case Level::Medium:
        return mixer_ctl::LogPriority::Medium;
    case Level::High:
        return mixer_ctl::LogPriority::High;
    case Level::Critical:
        return mixer_ctl::LogPriority::Critical;
    }
    throw Exception("Wrong log level value: " + std::to_string(static_cast<uint32_t>(level)));
}

Logger::Level Logger::translateFromMixer(mixer_ctl::LogPriority level)
{
    switch (level) {
    case mixer_ctl::LogPriority::Verbose:
        return Level::Verbose;
    case mixer_ctl::LogPriority::Low:
        return Level::Low;
    case mixer_ctl::LogPriority::Medium:
        return Level::Medium;
    case mixer_ctl::LogPriority::High:
        return Level::High;
    case mixer_ctl::LogPriority::Critical:
        return Level::Critical;
    case mixer_ctl::LogPriority::Quiet:
        // WTF???
        return Level::Verbose;
    }
    throw Exception("Wrong Mixer log level value: " + std::to_string(static_cast<uint32_t>(level)));
}
}
}
}
