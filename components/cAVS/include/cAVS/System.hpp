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
#pragma once

#include "cAVS/Driver.hpp"
#include "cAVS/DriverFactory.hpp"
#include <memory>
#include <stdexcept>
#include <vector>

namespace debug_agent
{
namespace cavs
{

/**
 * The cAVS System
 */
class System final
{
public:
    class Exception : public std::logic_error
    {
    public:
        explicit Exception(const std::string& what)
        : std::logic_error(what)
        {}
    };

    /**
     * @throw System::Exception
     */
    System(const DriverFactory &driverFactory);

    /**
     * Set log parameters
     * @param[in] parameters Log parameters to be set
     */
    void setLogParameters(Logger::Parameters &parameters);

    /**
     * Get log parameters
     * @return current log parameters
     */
    Logger::Parameters getLogParameters();

    /**
     * Get module entries
     */
    void getModuleEntries(std::vector<dsp_fw::ModuleEntry> &modulesEntries);

    /**
     * Streams out log in IFDK:cavs:fwlog format
     * @param[in] os the std::ostream where the log has to be written to
     */
    void doLogStream(std::ostream &os);

private:
    /* Make this class non copyable */
    System(const System &) = delete;
    System & operator=(const System &) = delete;

    static std::unique_ptr<Driver> createDriver(const DriverFactory &driverFactory);

    std::unique_ptr<Driver> mDriver;
};

}
}


