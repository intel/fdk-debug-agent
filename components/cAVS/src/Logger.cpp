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
#include <cAVS/Logger.hpp>

namespace debug_agent
{
namespace cavs
{

const std::string &Logger::toString(Output output)
{
    switch (output)
    {
    case Output::Sram:
        return sram;
    case Output::Pti:
        return pti;
    }
    // Unreachable
    std::abort();
}

const std::string &Logger::toString(Level level)
{
    switch (level)
    {
    case Level::Critical:
        return critical;
        break;
    case Level::High:
        return high;
        break;
    case Level::Medium:
        return medium;
        break;
    case Level::Low:
        return low;
        break;
    case Level::Verbose:
        return verbose;
        break;
    }
    // Unreachable
    std::abort();
}

Logger::Output Logger::outputFromString(const std::string &output)
{
    if (output == sram) {
        return Output::Sram;
    } else if (output == pti) {
        return Output::Pti;
    }
    throw Exception("Wrong log output value: " + output);
}

Logger::Level Logger::levelFromString(const std::string &level)
{
    if (level == critical) {
        return Level::Critical;
    } else if (level == high) {
        return Level::High;
    } else if (level == medium) {
        return Level::Medium;
    } else if (level == low) {
        return Level::Low;
    } else if (level == verbose) {
        return Level::Verbose;
    }
    throw Exception("Wrong log level value: " + level);
}

const std::string Logger::critical("Critical");
const std::string Logger::high("High");
const std::string Logger::medium("Medium");
const std::string Logger::low("Low");
const std::string Logger::verbose("Verbose");

const std::string Logger::sram("SRAM");
const std::string Logger::pti("PTI");

};
};
