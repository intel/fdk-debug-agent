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
#include <cAVS/Windows/Logger.hpp>
#include <cAVS/Windows/DriverTypes.hpp>
#include <string>
#include <cstring>
#include <algorithm>
/** @todo remove these header inclusion only needed for fake/demo log */
#include <thread>
#include <chrono>

/** Unfortunetely windows.h define "min" as macro, making fail the call std::min()
 * So undefining it */
#undef min

namespace debug_agent
{
namespace cavs
{
namespace windows
{

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
    TinyCmdLogParameterIoctl content;

    /* Seting supplied parameters.
     * May throws Logger::Exception */
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
    if (status != STATUS_SUCCESS) {
        throw Exception("Driver returns invalid status: " +
            std::to_string(static_cast<uint32_t>(status)));
    }
}

std::unique_ptr<LogBlock> Logger::readLogBlock()
{
    /** @todo read FW log using windows driver interface once available */
    static const std::string msg("Fake log from Windows driver");
    static const int fakeCoreID = 0x0F; // a 0x0F is a valid core ID and easy to see in hex editor

    std::unique_ptr<LogBlock> block(new LogBlock(fakeCoreID, msg.size()));
    std::copy(msg.begin(), msg.end(), block->getLogData().begin());

    /* Bandwidth limiter !
     * We must add a tempo for fake log since it would produce an infinite log stream bandwidth,
     * only limited by machine capacity. */
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return block;
}

}
}
}
