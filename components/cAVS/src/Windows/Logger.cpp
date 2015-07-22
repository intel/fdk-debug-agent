/*
********************************************************************************
*                              INTEL CONFIDENTIAL
*   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
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
#include "cAVS/Windows/Logger.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "Util/PointerHelper.hpp"
#include <string>
#include <cstring>
#include <algorithm>
#include <assert.h>

/** Unfortunetely windows.h define "min" as macro, making fail the call std::min()
 * So undefining it */
#undef min

namespace debug_agent
{
namespace cavs
{
namespace windows
{

Logger::LogProducer::LogProducer(BlockingQueue &queue, std::unique_ptr<WppClient> wppClient) :
    mQueue(queue),
    mWppClient(std::move(wppClient)),

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
        mWppClient->collectLogEntries(*this);
    }
    catch (WppClient::Exception &e)
    {
        /* Swallowing the exception because currently the SwAS doesn't propose any way
         * to forward the error to the client (here the client may be disconnected)
         */
        std::cout << "Error: can not collect log entries: " << e.what() << std::endl;
    }
}

void Logger::LogProducer::onLogEntry(uint32_t coreId, uint8_t *buffer,
    uint32_t bufferSize)
{
    std::unique_ptr<LogBlock> logBlock(new LogBlock(coreId, bufferSize));
    std::copy(buffer, buffer + bufferSize, logBlock->getLogData().begin());

    if (!mQueue.add(std::move(logBlock))) {
        std::cout << "Warning: dropping log entry: the queue is full or closed"
                  << std::endl;
    }
}

uint32_t Logger::getIoControlCodeFromType(IoCtlType type)
{
    switch (type)
    {
    case IoCtlType::Get:
        return IOCTL_CMD_APP_TO_AUDIODSP_TINY_GET;
    case IoCtlType::Set:
        return IOCTL_CMD_APP_TO_AUDIODSP_TINY_SET;
    }
    throw Exception("Wrong ioctl type value: " + std::to_string(static_cast<uint32_t>(type)));
}

std::string Logger::getIoControlTypeName(IoCtlType type)
{
    switch (type)
    {
    case IoCtlType::Get:
        return "TinyGet";
    case IoCtlType::Set:
        return "TinySet";
    }
    throw Exception("Wrong ioctl type value: " + std::to_string(static_cast<uint32_t>(type)));
}

driver::LOG_STATE Logger::translateToDriver(bool isStarted)
{
    return isStarted ? driver::LOG_STATE::STARTED : driver::LOG_STATE::STOPPED;
}

driver::LOG_LEVEL Logger::translateToDriver(Level level)
{
    switch (level)
    {
    case Level::Verbose:
        return driver::LOG_LEVEL::VERBOSE;
    case Level::Low:
        return driver::LOG_LEVEL::LOW;
    case Level::Medium:
        return driver::LOG_LEVEL::MEDIUM;
    case Level::High:
        return driver::LOG_LEVEL::HIGH;
    case Level::Critical:
        return driver::LOG_LEVEL::CRITICAL;
    }
    throw Exception("Wrong log level value: " + std::to_string(static_cast<uint32_t>(level)));
}

driver::LOG_OUTPUT Logger::translateToDriver(Output output)
{
    switch (output)
    {
    case Output::Pti:
        return driver::LOG_OUTPUT::OUTPUT_PTI;
    case Output::Sram:
        return driver::LOG_OUTPUT::OUTPUT_SRAM;
    }
    throw Exception("Wrong log output value: " + std::to_string(static_cast<uint32_t>(output)));
}

bool Logger::translateFromDriver(driver::LOG_STATE state)
{
    switch (state)
    {
    case driver::LOG_STATE::STARTED:
        return true;
    case driver::LOG_STATE::STOPPED:
        return false;
    }
    throw Exception("Wrong driver log state value: " +
        std::to_string(static_cast<uint32_t>(state)));
}

Logger::Level Logger::translateFromDriver(driver::LOG_LEVEL level)
{
    switch (level)
    {
    case driver::LOG_LEVEL::VERBOSE:
        return Level::Verbose;
    case driver::LOG_LEVEL::LOW:
        return Level::Low;
    case driver::LOG_LEVEL::MEDIUM:
        return Level::Medium;
    case driver::LOG_LEVEL::HIGH:
        return Level::High;
    case driver::LOG_LEVEL::CRITICAL:
        return Level::Critical;
    }
    throw Exception("Wrong driver log level value: " +
        std::to_string(static_cast<uint32_t>(level)));
}

Logger::Output Logger::translateFromDriver(driver::LOG_OUTPUT output)
{
    switch (output)
    {
    case driver::LOG_OUTPUT::OUTPUT_PTI:
        return Output::Pti;
    case driver::LOG_OUTPUT::OUTPUT_SRAM:
        return Output::Sram;
    }
    throw Exception("Wrong driver log output value: " +
        std::to_string(static_cast<uint32_t>(output)));
}

driver::FwLogsState Logger::translateToDriver(const Parameters& params)
{
    driver::FwLogsState fwParams = {
        translateToDriver(params.mIsStarted),
        translateToDriver(params.mLevel),
        translateToDriver(params.mOutput)
    };
    return fwParams;
}

Logger::Parameters Logger::translateFromDriver(const driver::FwLogsState &fwParams)
{
    Parameters params = {
        translateFromDriver(static_cast<driver::LOG_STATE>(fwParams.started)),
        translateFromDriver(static_cast<driver::LOG_LEVEL>(fwParams.level)),
        translateFromDriver(static_cast<driver::LOG_OUTPUT>(fwParams.output))
    };
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
        }
        else {
            stopLogLocked(parameters);
        }
    }
    else {
        /* Start/Stop state has not changed */

        if (isLogProductionRunning) {
            throw Exception("Can not change log parameters while logging is activated.");
        }
        else {
            updateLogLocked(parameters);
        }
    }
}

void Logger::startLogLocked(const Parameters &parameters)
{
    assert(mLogProducer == nullptr);

    /* Starting the producer thread before enabling logs into the fw */
    mLogProducer = std::move(std::make_unique<LogProducer>(mLogEntryQueue,
        mWppClientFactory.createInstance()));

    try
    {
        setLogParameterIoctl(parameters);
    }
    catch (Exception &)
    {
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
    TinyCmdLogParameterIoctl content;

    /* Seting supplied parameters.
    * May throw Logger::Exception */
    content.getFwLogsState() = translateToDriver(parameters);

    /* Performing the ioctl */
    logParameterIoctl(IoCtlType::Set, content);
}

Logger::Parameters Logger::getParameters()
{
    TinyCmdLogParameterIoctl content;

    /* Performing the ioctl */
    logParameterIoctl(IoCtlType::Get, content);

    /* Returning the parameters
     * May throws Logger::Exception */
    return translateFromDriver(content.getFwLogsState());
}

void Logger::logParameterIoctl(IoCtlType type, TinyCmdLogParameterIoctl &content)
{
    uint32_t ioControlCode = getIoControlCodeFromType(type);

    try
    {
        mDevice.ioControl(ioControlCode, &content.getBuffer(), &content.getBuffer());
    }
    catch (Device::Exception &e)
    {
        throw Exception(getIoControlTypeName(type) + " error: " + e.what());
    }

    NTSTATUS status = content.getTinyCmd().Body.Status;
    if (!NT_SUCCESS(status)) {
        throw Exception("Driver returns invalid status: " +
            std::to_string(static_cast<uint32_t>(status)));
    }
}

std::unique_ptr<LogBlock> Logger::readLogBlock()
{
    return mLogEntryQueue.remove();
}

void Logger::stop()
{
    /* Stopping log session if one is running */
    std::lock_guard<std::mutex> locker(mLogActivationContextMutex);
    if (mLogProducer != nullptr) {

        /* Using arbitrary level and output values, they are not relevant when stopping log... */
        Parameters parameter(false, Logger::Level::Verbose, Logger::Output::Sram);
        try
        {
            stopLogLocked(parameter);
        }
        catch (Exception &e)
        {
            std::cout << "Cannot stop log producer : " << e.what() << std::endl;
        }
    }

    /* Unblocking log consumer threads */
    mLogEntryQueue.close();
}

}
}
}
