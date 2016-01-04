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
#include <cAVS/Linux/Logger.hpp>
#include <string>
#include <cstring>
#include <algorithm>
/** @todo remove these header inclusion only needed for fake/demo log */
#include <thread>
#include <chrono>

namespace debug_agent
{
namespace cavs
{
namespace linux
{

void Logger::setParameters(const Parameters &parameters)
{
    /** @todo set parameters in driver through the Debug FS interface */
    mDriverEmulationParameter = parameters;
}

Logger::Parameters Logger::getParameters()
{
    /** @todo set parameters using the driver Debug FS interface */
    return mDriverEmulationParameter;
}

std::unique_ptr<LogBlock> Logger::readLogBlock()
{
    /** @todo read FW log using linux driver interface once available */
    static const std::string msg("Fake log from Linux driver");
    static const int fakeCoreID = 0x0F; // a 0x0F is a valid core ID and easy to see in hex editor

    std::unique_ptr<LogBlock> block = std::make_unique<LogBlock>(fakeCoreID, msg.size());
    std::copy(msg.begin(), msg.end(), block->getLogData().begin());

    /* Bandwidth limiter !
     * We must add a tempo for fake log since it would produce an infinite log stream bandwidth,
     * only limited by machine capacity. */
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return block;
}

void Logger::stop() NOEXCEPT
{
    /* To do */
}
}
}
}
