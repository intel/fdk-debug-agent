/*
 * Copyright (c) 2015, Intel Corporation
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
#include "cAVS/Windows/Logger.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/IoctlHelpers.hpp"
#include "Util/PointerHelper.hpp"
#include "Util/ByteStreamReader.hpp"
#include "Util/ByteStreamWriter.hpp"
#include <string>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <iostream>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

Logger::LogProducer::LogProducer(BlockingQueue &queue, std::unique_ptr<WppClient> wppClient)
    : mQueue(queue), mWppClient(std::move(wppClient)),

      /* Note: mProducerThread should be the last initialized member because it starts a thread
       * that uses the previous members.
       */
      mProducerThread(&Logger::LogProducer::produceEntries, this)
{
    assert(mWppClient != nullptr);
}

Logger::LogProducer::~LogProducer()
{
    mWppClient->stop();
    mProducerThread.join();
}

void Logger::LogProducer::produceEntries()
{
    try {
        // Queue is open during collectLogEntries() call
        BlockingQueue::AutoOpenClose closer(mQueue);
        mWppClient->collectLogEntries(*this);
    } catch (WppClient::Exception &e) {
        /* Swallowing the exception because currently the SwAS doesn't propose any way
         * to forward the error to the client (here the client may be disconnected)
         */
        std::cout << "Error: can not collect log entries: " << e.what() << std::endl;
    }
}

void Logger::LogProducer::onLogEntry(uint32_t coreId, uint8_t *buffer, uint32_t bufferSize)
{
    std::unique_ptr<LogBlock> logBlock = std::make_unique<LogBlock>(coreId, bufferSize);
    std::copy(buffer, buffer + bufferSize, logBlock->getLogData().begin());

    if (!mQueue.add(std::move(logBlock))) {
        std::cout << "Warning: dropping log entry: the queue is full or closed" << std::endl;
    }
}

driver::IOCTL_LOG_STATE Logger::translateToDriver(bool isStarted)
{
    return isStarted ? driver::IOCTL_LOG_STATE::STARTED : driver::IOCTL_LOG_STATE::STOPPED;
}

driver::FW_LOG_LEVEL Logger::translateToDriver(Level level)
{
    switch (level) {
    case Level::Verbose:
        return driver::FW_LOG_LEVEL::LOG_VERBOSE;
    case Level::Low:
        return driver::FW_LOG_LEVEL::LOG_LOW;
    case Level::Medium:
        return driver::FW_LOG_LEVEL::LOG_MEDIUM;
    case Level::High:
        return driver::FW_LOG_LEVEL::LOG_HIGH;
    case Level::Critical:
        return driver::FW_LOG_LEVEL::LOG_CRITICAL;
    }
    throw Exception("Wrong log level value: " + std::to_string(static_cast<uint32_t>(level)));
}

driver::FW_LOG_OUTPUT Logger::translateToDriver(Output output)
{
    switch (output) {
    case Output::Pti:
        return driver::FW_LOG_OUTPUT::OUTPUT_PTI;

    /** @todo: remove this workaround : now translating Output::Sram --> FW_LOG_OUTPUT::OUTPUT_WPP
     */
    case Output::Sram:
        return driver::FW_LOG_OUTPUT::OUTPUT_WPP;
    }
    throw Exception("Wrong log output value: " + std::to_string(static_cast<uint32_t>(output)));
}

bool Logger::translateFromDriver(driver::IOCTL_LOG_STATE state)
{
    switch (state) {
    case driver::IOCTL_LOG_STATE::STARTED:
        return true;
    case driver::IOCTL_LOG_STATE::STOPPED:
        return false;
    }
    throw Exception("Wrong driver log state value: " +
                    std::to_string(static_cast<uint32_t>(state)));
}

Logger::Level Logger::translateFromDriver(driver::FW_LOG_LEVEL level)
{
    switch (level) {
    case driver::FW_LOG_LEVEL::LOG_VERBOSE:
        return Level::Verbose;
    case driver::FW_LOG_LEVEL::LOG_LOW:
        return Level::Low;
    case driver::FW_LOG_LEVEL::LOG_MEDIUM:
        return Level::Medium;
    case driver::FW_LOG_LEVEL::LOG_HIGH:
        return Level::High;
    case driver::FW_LOG_LEVEL::LOG_CRITICAL:
        return Level::Critical;
    }
    throw Exception("Wrong driver log level value: " +
                    std::to_string(static_cast<uint32_t>(level)));
}

Logger::Output Logger::translateFromDriver(driver::FW_LOG_OUTPUT output)
{
    switch (output) {
    case driver::FW_LOG_OUTPUT::OUTPUT_PTI:
        return Output::Pti;
    /** @todo: remove this workaround : now translating FW_LOG_OUTPUT::OUTPUT_WPP --> Output::Sram
     */
    case driver::FW_LOG_OUTPUT::OUTPUT_WPP:
        return Output::Sram;
    }
    throw Exception("Wrong driver log output value: " +
                    std::to_string(static_cast<uint32_t>(output)));
}

driver::IoctlFwLogsState Logger::translateToDriver(const Parameters &params)
{
    driver::IoctlFwLogsState fwParams = {translateToDriver(params.mIsStarted),
                                         translateToDriver(params.mLevel),
                                         translateToDriver(params.mOutput)};
    return fwParams;
}

Logger::Parameters Logger::translateFromDriver(const driver::IoctlFwLogsState &fwParams)
{
    Parameters params = {
        translateFromDriver(static_cast<driver::IOCTL_LOG_STATE>(fwParams.started)),
        translateFromDriver(static_cast<driver::FW_LOG_LEVEL>(fwParams.level)),
        translateFromDriver(static_cast<driver::FW_LOG_OUTPUT>(fwParams.output))};
    return params;
}

void Logger::setParameters(const Parameters &parameters)
{
    std::lock_guard<std::mutex> locker(mLogActivationContextMutex);

    bool isLogProductionRunning = mLogProducer != nullptr;
    if (isLogProductionRunning != parameters.mIsStarted) {
        /* Start/Stop state has changed */

        if (parameters.mIsStarted) {
            startLogLocked(parameters);
        } else {
            stopLogLocked(parameters);
        }
    } else {
        /* Start/Stop state has not changed */

        if (isLogProductionRunning) {
            throw Exception("Can not change log parameters while logging is activated.");
        } else {
            updateLogLocked(parameters);
        }
    }
}

void Logger::startLogLocked(const Parameters &parameters)
{
    assert(mLogProducer == nullptr);

    /* Starting the producer thread before enabling logs into the fw */
    mLogProducer = std::move(
        std::make_unique<LogProducer>(mLogEntryQueue, mWppClientFactory.createInstance()));

    try {
        setLogParameterIoctl(parameters);
    } catch (Exception &) {
        /* Stopping the log producer thread in exception case */
        mLogProducer.reset();
        throw;
    }
}

void Logger::stopLogLocked(const Parameters &parameters)
{
    assert(mLogProducer != nullptr);

    /* This ensures that mLogProducer will be deleted, even if an exception is thrown */
    util::EnsureUniquePtrDeletion<LogProducer> ptrDeleter(mLogProducer);

    /* May throw Logger::Exception */
    setLogParameterIoctl(parameters);
}

void Logger::updateLogLocked(const Parameters &parameters)
{
    /* Cannot change log parameters during running session */
    assert(mLogProducer == nullptr);

    setLogParameterIoctl(parameters);
}

void Logger::setLogParameterIoctl(const Parameters &parameters)
{
    /* Seting supplied parameters.
    * May throw Logger::Exception */
    driver::IoctlFwLogsState inputFwParams = translateToDriver(parameters);

    /* Unused, nothing is returned when setting log parameters */
    driver::IoctlFwLogsState outputParams;

    /* Performing the ioctl */
    logParameterIoctl(Direction::Set, inputFwParams, outputParams);
}

Logger::Parameters Logger::getParameters()
{
    /** Initializing inputParams with invalid values, to ensure that they will be overwritten by
     * the driver */
    driver::IoctlFwLogsState inputFwParams = {
        static_cast<driver::IOCTL_LOG_STATE>(0xFFFFFFFF),
        static_cast<driver::FW_LOG_LEVEL>(0xFFFFFFFF),
        static_cast<driver::FW_LOG_OUTPUT>(0xFFFFFFFF),
    };
    driver::IoctlFwLogsState outputFwParams;

    /* Performing the ioctl */
    logParameterIoctl(Direction::Get, inputFwParams, outputFwParams);

    /* Returning the parameters
     * May throws Logger::Exception */
    return translateFromDriver(outputFwParams);
}

void Logger::logParameterIoctl(Direction direction, const driver::IoctlFwLogsState &inputFwParams,
                               driver::IoctlFwLogsState &outputFwParams)
{
    /* Creating the body payload using the IoctlFwLogsState type */
    util::MemoryByteStreamWriter bodyPayloadWriter;
    bodyPayloadWriter.write(inputFwParams);

    /* Creating the TinySet/Get ioctl buffer */
    util::Buffer buffer = ioctl_helpers::toTinyCmdBuffer(
        static_cast<uint32_t>(driver::IOCTL_FEATURE::FEATURE_FW_LOGS),
        driver::logParametersCommandparameterId, bodyPayloadWriter.getBuffer());

    driver::IoCtlType type =
        direction == Direction::Set ? driver::IoCtlType::TinySet : driver::IoCtlType::TinyGet;
    /* Performing ioctl */
    try {
        mDevice.ioControl(type, &buffer, &buffer);
    } catch (Device::Exception &e) {
        throw Exception(to_string(type) + " error: " + e.what());
    }

    try {
        NTSTATUS driverStatus;
        util::Buffer bodyPayloadBuffer;

        /* Parsing returned buffer */
        std::tie(driverStatus, bodyPayloadBuffer) = ioctl_helpers::fromTinyCmdBuffer(buffer);

        if (!NT_SUCCESS(driverStatus)) {
            throw Exception("Driver returns invalid status: " +
                            std::to_string(static_cast<uint32_t>(driverStatus)));
        }

        /* Reading IoctlFwLogsState structure from body payload */
        util::MemoryByteStreamReader reader(bodyPayloadBuffer);
        reader.read(outputFwParams);

        if (!reader.isEOS()) {
            /** @todo use logging or throw an exception */
            std::cout << "Log parameter ioctl buffer has not been fully consumed,"
                      << " IsGet=" << ((direction == Direction::Get) ? true : false)
                      << " pointer=" << reader.getPointerOffset()
                      << " size=" << reader.getBuffer().size()
                      << " remaining= " << (reader.getBuffer().size() - reader.getBuffer().size());
        }
    } catch (util::ByteStreamReader::Exception &e) {
        throw Exception("Can not decode log parameter ioctl buffer: " + std::string(e.what()));
    }
}

std::unique_ptr<LogBlock> Logger::readLogBlock()
{
    return mLogEntryQueue.remove();
}

void Logger::stop() noexcept
{
    /* Stopping log session if one is running */
    std::lock_guard<std::mutex> locker(mLogActivationContextMutex);
    if (mLogProducer != nullptr) {

        /* Using arbitrary level and output values, they are not relevant when stopping log... */
        Parameters parameter(false, Logger::Level::Verbose, Logger::Output::Sram);
        try {
            stopLogLocked(parameter);
        } catch (Exception &e) {
            std::cout << "Cannot stop log producer : " << e.what() << std::endl;
        }
    }

    /* Unblocking log consumer threads */
    mLogEntryQueue.close();
}
}
}
}
