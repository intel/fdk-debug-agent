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

#include <cAVS/Logger.hpp>
#include <System/IfdkStreamer.hpp>
#include <ostream>
#include <string>
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{

class LogStreamer final: public system::IfdkStreamer
{
public:
    /**
     * A LogStreamer writes the cAVS log to an ostream in real time, using a cavs::Logger interface.
     * @param[in] logger The Logger interface to be used to get the cAVS log
     * @throw LogStreamer::Exception
     * @todo The LogStreamer will need a way to retrieve the "Module Entries" table in a subsequent
     * patch.
     */
    LogStreamer(Logger &logger);

private:
    virtual void streamFormatHeader(std::ostream &os) override;
    virtual bool streamNextFormatData(std::ostream &os) override;

    /**
     * The Logger interface to be used to get the cAVS log
     */
    Logger &mLogger;

    /**
     * The stream is produced in the IFDK stream format which requires a system type (cavs)
     * @todo this system type should be used in multiple places in cAVS in the near future,
     * consider to define it somewhere else reachable from cavs::LogStreamer.
     */
    static const std::string systemType;

    /**
     * The stream is produced in the IFDK stream format which requires a format type (fwlog)
     */
    static const std::string formatType;

    /**
     * The IFDK:cavs:fwlog format major version
     */
    static const int majorVersion;

    /**
     * The IFDK:cavs:fwlog format minor version
     */
    static const int minorVersion;

    /* Make this class non copyable */
    LogStreamer(const LogStreamer &) = delete;
    LogStreamer & operator=(const LogStreamer &) = delete;
};

}
}