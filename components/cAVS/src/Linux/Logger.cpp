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

namespace debug_agent
{
namespace cavs
{
namespace linuxx
{

void Logger::setParameters(Parameters &parameters)
{
    /** @todo set parameters in driver through the Debug FS interface */
    mDriverEmulationParameter = parameters;
}

Logger::Parameters Logger::getParameters()
{
    /** @todo set parameters using the driver Debug FS interface */
    return mDriverEmulationParameter;
}

std::size_t Logger::read(void *buf, std::size_t count)
{
    /** @todo read FW log using driver Debug FS interface once available */
    static const std::string msg("Log from Linux driver\n");
    std::size_t len = std::min(count, msg.length());
    std::memcpy(buf, msg.c_str(), len);
    return len;
}

}
}
}
