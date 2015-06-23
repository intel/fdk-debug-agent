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
#include "cAVS/System.hpp"
#include "cAVS/DriverFactory.hpp"
#include <utility>

namespace debug_agent
{
namespace cavs
{

System::System()
: mDriver(std::move(createDriver()))
{
    if (mDriver == nullptr) {

        throw Exception("Driver factory has failed");
    }
}

std::unique_ptr<Driver> System::createDriver()
{
    try
    {
        return DriverFactory::newDriver();
    }
    catch (DriverFactory::Exception e)
    {
        throw Exception("Unable to create driver: " + std::string(e.what()));
    }
}

void System::setLogParameters(Logger::Parameters &parameters)
{
    try
    {
        mDriver->getLogger().setParameters(parameters);
    }
    catch (Logger::Exception &e)
    {
        throw Exception("Unable to set log parameter: " + std::string(e.what()));
    }
}

Logger::Parameters System::getLogParameters()
{
    try
    {
        return mDriver->getLogger().getParameters();
    }
    catch (Logger::Exception &e)
    {
        throw Exception("Unable to get log parameter: " + std::string(e.what()));
    }
}

}
}


