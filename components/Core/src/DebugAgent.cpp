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

#include "Core/DebugAgent.hpp"
#include "Core/Resources.hpp"
#include <memory>
#include <exception>

using namespace debug_agent::rest;

namespace debug_agent
{
namespace core
{

std::shared_ptr<rest::Dispatcher> DebugAgent::createDispatcher()
{
    Dispatcher *dispatcher = new rest::Dispatcher();

    dispatcher->addResource("/cAVS/logging/stream",
        std::shared_ptr<Resource>(new LogStreamResource(mSystem)));
    dispatcher->addResource("/cAVS/logging/parameters",
        std::shared_ptr<Resource>(new LogParametersResource(mSystem)));
    dispatcher->addResource("/cAVS/module/entries",
        std::shared_ptr<Resource>(new ModuleEntryResource(mSystem)));

    return std::shared_ptr<rest::Dispatcher>(dispatcher);
}

DebugAgent::DebugAgent(const cavs::DriverFactory &driverFactory, uint32_t port)
try : mSystem(driverFactory), mRestServer(createDispatcher(), port)
{
}
catch (rest::Dispatcher::InvalidUriException &e)
{
    throw Exception("Invalid resource URI: " + std::string(e.what()));
}
catch (rest::Server::Exception &e)
{
    throw Exception("Rest server error : " + std::string(e.what()));
}
catch (cavs::System::Exception &e)
{
    throw Exception("System error: " + std::string(e.what()));
}

DebugAgent::~DebugAgent()
{
    /* This call will unblock all threads that consume system events (log...) */
    mSystem.stop();

   /* Then rest server destructor can terminate the http request threads gracefully */
}


}
}
