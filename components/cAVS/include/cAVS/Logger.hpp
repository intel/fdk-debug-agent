/*
 * Copyright (c) 2015-2016, Intel Corporation
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
#pragma once

#include "Util/EnumHelper.hpp"
#include "cAVS/LogBlock.hpp"
#include <stdexcept>
#include <string>
#include <memory>

namespace debug_agent
{
namespace cavs
{

/**
 * Defines the cAVS::Logger interface used to control and read cAVS FW log abstracting the
 * DSP driver which differs on Windows and Linux.
 */
class Logger
{
public:
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
    };

    /**
     * Firmware log output is PTI hardware bus or SRAM to be captured
     * by the driver.
     */
    enum class Output
    {
        Sram,
        Pti
    };

    static const util::EnumHelper<Output> &outputHelper()
    {
        static const util::EnumHelper<Output> helper({
            {Output::Sram, "SRAM"}, {Output::Pti, "PTI"},
        });
        return helper;
    }

    /**
     * Log level. This enumeration matches the cAVS FW Log Level definition.
     */
    enum class Level : uint32_t
    {
        Critical = 1,
        High,
        Medium,
        Low,
        Verbose
    };

    static const util::EnumHelper<Level> &levelHelper()
    {
        static const util::EnumHelper<Level> helper({
            {Level::Critical, "Critical"},
            {Level::High, "High"},
            {Level::Medium, "Medium"},
            {Level::Low, "Low"},
            {Level::Verbose, "Verbose"},
        });
        return helper;
    }

    /**
     * The parameters of a log
     */
    struct Parameters
    {

        Parameters(bool isStarted, Level level, Output output)
            : mIsStarted(isStarted), mLevel(level), mOutput(output)
        {
        }

        Parameters() : mIsStarted(false), mLevel(Level::Verbose), mOutput(Output::Sram){};

        bool operator==(const Parameters &other) const
        {
            return mIsStarted == other.mIsStarted && mLevel == other.mLevel &&
                   mOutput == other.mOutput;
        }

        /**
         * Log state: started if true, stopped if false
         */
        bool mIsStarted;
        Level mLevel;
        Output mOutput;
    };

    /**
     * Instantiates a cAVS Logger
     */
    Logger() = default;

    virtual ~Logger() {}

    /**
     * Returns the log parameters
     * @return the log parameters
     */
    virtual Parameters getParameters() = 0;

    /**
     * Set the log parameters
     * @param[in] parameters the log parameters to be set
     * @throw Logger::Exception
     */
    virtual void setParameters(const Parameters &parameters) = 0;

    /**
     * Read the FW log stream.
     * Each time the method is called, each time it returns the next log block of the log stream.
     * @remarks The method waits until FW log data are available.
     * @return a block of log
     */
    virtual std::unique_ptr<LogBlock> readLogBlock() = 0;

    /**
    * Stop internal threads and unblock consumer threads
    */
    virtual void stop() noexcept = 0;

private:
    /* Make this class non copyable */
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    /** Level enum class string representation */
    static const std::string critical;
    static const std::string high;
    static const std::string medium;
    static const std::string low;
    static const std::string verbose;

    /** Output enum class string representation */
    static const std::string sram;
    static const std::string pti;
};
};
};
