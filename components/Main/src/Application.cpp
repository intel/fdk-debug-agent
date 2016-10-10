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

#include "Main/Application.hpp"
#include "Util/About.hpp"
#include "cAVS/SystemDriverFactory.hpp"
#include "Core/DebugAgent.hpp"
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/IntValidator.h>
#include <cassert>

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

void Application::usage()
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("OPTIONS");
    helpFormatter.setHeader("Provides the ability to debug firmwares.");
    helpFormatter.setUnixStyle(true);
    helpFormatter.format(std::cout);
}

void Application::handleHelp(const std::string &, const std::string &)
{
    mConfig.helpRequested = true;
    usage();
    stopOptionsProcessing();
}

void Application::handlePort(const std::string &, const std::string &value)
{
    /** @fixme use Convert */
    std::stringstream ss(value);
    ss >> mConfig.serverPort;
    assert((!ss.fail()) && (!ss.bad()));
}

void Application::handlePfwConfig(const std::string &, const std::string &value)
{
    mConfig.pfwConfig = value;
}

void Application::handleLogControlOnly(const std::string &, const std::string &)
{
    mConfig.logControlOnly = true;
}

void Application::handleVerbose(const std::string &, const std::string &)
{
    mConfig.serverIsVerbose = true;
}

void Application::handleValidation(const std::string &, const std::string &)
{
    mConfig.validationRequested = true;
}

void Application::handleVersion(const std::string &, const std::string &)
{
    std::cout << util::about::version() << std::endl;
    std::exit(0);
}

void Application::defineOptions(OptionSet &options)
{
    options.addOption(Option("help", "h", "Display DebugAgent help.")
                          .required(false)
                          .repeatable(false)
                          .callback(OptionCallback<Application>(this, &Application::handleHelp)));

    options.addOption(Option("port", "p", "Set HTTP server port")
                          .required(false)
                          .repeatable(false)
                          .argument("value")

                          /* Poco forces us to use operator new here: the Option takes the ownership
                           * of the IntValidator.
                           */
                          .validator(new IntValidator(22, 65535))
                          .callback(OptionCallback<Application>(this, &Application::handlePort)));

    options.addOption(
        Option("pfwConfig", "pf", "Set configuration file for parameter-framework instance")
            .required(true)
            .repeatable(false)
            .argument("value")
            .callback(OptionCallback<Application>(this, &Application::handlePfwConfig)));

    options.addOption(
        Option("control", "c", "Disable CAVS FW log data path, and keep CAVS FW log control "
                               "capabilities of DebugAgent")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<Application>(this, &Application::handleLogControlOnly)));

    options.addOption(
        Option("verbose", "v", "Enable verbose logging")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<Application>(this, &Application::handleVerbose)));

    options.addOption(
        Option("validate", "x", "Validate XML files")
            .required(false)
            .repeatable(false)
            .callback(OptionCallback<Application>(this, &Application::handleValidation)));

    options.addOption(
        Option("version", "", "Print the DebugAgent's version and exit")
            .required(false)
            .callback(OptionCallback<Application>(this, &Application::handleVersion)));
}

int Application::main(const std::vector<std::string> &)
{
    if (mConfig.helpRequested) {
        return Application::EXIT_OK;
    }

    try {
        SystemDriverFactory driverFactory(mConfig.logControlOnly);
        DebugAgent debugAgent(driverFactory, mConfig.serverPort, mConfig.pfwConfig,
                              mConfig.serverIsVerbose, mConfig.validationRequested);

        std::cout << "DebugAgent started" << std::endl;

        waitForTerminationRequest(); /* wait for CTRL-C or kill */

        std::cout << std::endl << "Shutting down DebugAgent..." << std::endl;
    } catch (DebugAgent::Exception &e) {
        std::cout << "DebugAgent exception: " << e.what() << std::endl;
        return Application::EXIT_SOFTWARE;
    } catch (std::exception &e) {
        /* This block should not be reached */
        std::cout << "Unexpected std::exception of type '" << typeid(e).name() << "': " << e.what()
                  << std::endl;
        return Application::EXIT_SOFTWARE;
    }
    return Application::EXIT_OK;
}
}
}
