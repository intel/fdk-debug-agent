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
#include <string>
#include <cstring>
#include <algorithm>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

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
    }

    mLogLevel = parameters.mLevel;
    /** @todo: set the log level
     */
}

void Logger::startLogLocked(const Parameters & /*parameters*/)
{
    assert(!isLogProductionRunning());

    /* Clearing the log queue at session start and open it during all logger session. */
    mLogEntryQueue.clear();
    mLogEntryQueue.open();

    /* Starting the producer thread after setting level of logs into the fw */
    constructProducers();
}

void Logger::stopLogLocked(const Parameters & /*parameters*/)
{
    destroyProducers();

    /* Unblocking log consumer threads */
    mLogEntryQueue.close();
}

Logger::Parameters Logger::getParameters()
{
    /** Control Device exposes only a control to set the log level.
     * Log production is started once tinycompress devices
     * are opened and started
     */
    return {isLogProductionRunning(), mLogLevel, Output::Sram};
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

/* The constructor starts the log producer thread */
Logger::LogProducer::LogProducer(BlockingLogQueue &queue, unsigned int coreId, Device &device,
                                 std::unique_ptr<CompressDevice> logDevice)
    : mQueue(queue), mCoreId(coreId), mLogDevice(std::move(logDevice)), mDevice(device),
      mCorePower(device, coreId)
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
