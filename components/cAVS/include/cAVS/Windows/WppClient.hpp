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

#include "cAVS/Windows/WppLogEntryListener.hpp"
#include <stdexcept>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Base class of a wpp client */
class WppClient
{
public:
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    /** @throw WppClient::Exception (by subclasses) */
    WppClient() = default;
    virtual ~WppClient() = default;

    /** Collect entries until the stop() method is called by another thread.
     *
     * @throw WppClient::Exception
     */
    virtual void collectLogEntries(WppLogEntryListener &listener) = 0;

    /** Stop collecting.
     *
     * - if this method is called before collectEntries(), the next call to collectEntries()
     *   will return immediately.
     * - if this method is called during collectEntries(), collectEntries() ends.
     * - if this method is called after collectEntries(), it has no effect.
     */
    virtual void stop() NOEXCEPT = 0;

private:
    WppClient(const WppClient &) = delete;
    WppClient &operator=(const WppClient &) = delete;
};
}
}
}
