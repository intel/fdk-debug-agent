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
