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

#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/OptionSet.h>
#include <inttypes.h>

namespace debug_agent
{
namespace main
{

class Application : public Poco::Util::ServerApplication
{
public:
    Application();
    virtual ~Application(){};

protected:
    virtual void defineOptions(Poco::Util::OptionSet &options) override;

    virtual int main(const std::vector<std::string> &) override;

private:
    void usage();

    /* Option handlers */
    void handleHelp(const std::string &name, const std::string &value);
    void handlePort(const std::string &name, const std::string &value);
    void handlePfwConfig(const std::string &name, const std::string &value);
    void handleLogControlOnly(const std::string &name, const std::string &value);
    void handleVerbose(const std::string &name, const std::string &value);
    void handleValidation(const std::string &name, const std::string &value);
    void handleVersion(const std::string &name, const std::string &value);

    struct Config
    {
        bool helpRequested;
        uint32_t serverPort;
        std::string pfwConfig;
        bool logControlOnly;
        bool serverIsVerbose;
        bool validationRequested;
        Config()
            : helpRequested(false), serverPort(9090), logControlOnly(false), serverIsVerbose(false),
              validationRequested(false){};
    };

    Config mConfig;
};
}
}
