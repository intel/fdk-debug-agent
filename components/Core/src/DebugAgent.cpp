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
#include "Rest/Server.hpp"
#include "cAVS/Resources.hpp"
#include "cAVS/System.hpp"
#include <memory>

using namespace Poco::Util;
using namespace debug_agent::rest;

namespace debug_agent
{
namespace core
{

/**
 * @fixme This is a temporary port value
 */
const uint32_t DebugAgent::port = 9090;

int DebugAgent::main(const std::vector<std::string>&)
{
    // System is currently limited to cAVS
    cavs::System system;

    std::shared_ptr<rest::Dispatcher> dispatcher(new rest::Dispatcher());

    dispatcher->addResource("/cAVS/logging/stream",
        std::shared_ptr<Resource>(new cavs::LogStreamResource(system)));
    dispatcher->addResource("/cAVS/logging/parameters",
        std::shared_ptr<Resource>(new cavs::LogParametersResource(system)));

    {
        rest::Server restServer(dispatcher, port);

        std::cout << "DebugAgent started" << std::endl;

        waitForTerminationRequest();  /* wait for CTRL-C or kill */

        std::cout << std::endl << "Shutting down DebugAgent..." << std::endl;
    }

    return Application::EXIT_OK;
}

}
}
