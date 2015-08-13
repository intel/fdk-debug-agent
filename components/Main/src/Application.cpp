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

#include "Main/Application.hpp"
#include "cAVS/SystemDriverFactory.hpp"
#include "Core/DebugAgent.hpp"
#include <Poco/Util/HelpFormatter.h>

using namespace debug_agent::core;
using namespace debug_agent::cavs;
using namespace Poco::Util;

namespace debug_agent
{
namespace main
{

Application::Application()
{
    setUnixOptions(true);
}

void Application::usage(){
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("Provides the ability to debug firmwares.");
    helpFormatter.setUnixStyle(true);
    helpFormatter.format(std::cout);
}

void Application::handleHelp(const std::string& name, const std::string& value){
    mHelpRequested = true;
    usage();
    stopOptionsProcessing();
}

void Application::defineOptions(OptionSet& options)
{
    options.addOption(Option("help", "h", "Display DebugAgent help.")
    .required(false)
    .repeatable(false)
    .callback(OptionCallback<Application>(this, &Application::handleHelp)));
}

int Application::main(const std::vector<std::string>&)
{
    if (mHelpRequested){
        return Application::EXIT_OK;
    }

    try
    {
        SystemDriverFactory driverFactory;
        DebugAgent debugAgent(driverFactory, ServerPort);

        std::cout << "DebugAgent started" << std::endl;

        waitForTerminationRequest();  /* wait for CTRL-C or kill */

        std::cout << std::endl << "Shutting down DebugAgent..." << std::endl;
    }
    catch (DebugAgent::Exception &e)
    {
        std::cout << "DebugAgent exception: " << e.what() << std::endl;
        return Application::EXIT_SOFTWARE;
    }
    catch (std::exception &e)
    {
        /* This block should not be reached */
        std::cout << "Unexpected std::exception of type '" << typeid(e).name() << "': " << e.what()
            << std::endl;
        return Application::EXIT_SOFTWARE;
    }
    return Application::EXIT_OK;
}

}
}
