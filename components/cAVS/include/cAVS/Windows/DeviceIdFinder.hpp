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

#include "cAVS/Windows/WindowsTypes.hpp"
#include <stdexcept>
#include <string>
#include <set>
#include <vector>
#include <setupapi.h>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Find a windows device identifier from a GUID class and a substring of the device id */
class DeviceIdFinder final
{
public:
    class Exception : public std::logic_error
    {
    public:
        Exception(const std::string &msg) : std::logic_error(msg.c_str()) {}
    };

    /** Find all windows device identifiers matching a GUID class and containing a given substring
     * If there is no device id found, an exception is thrown
     *
     * @param[in] guid the GUID class to match
     * @param[out] deviceIds the found device identifiers
     * @param[in] substring the found devices identifiers have to contain this substring (optional)
     * @throw DeviceIdFinder::Exception
     */
    static void findAll(const GUID &guid, std::set<std::string> &deviceIds,
                        const std::string &substring = "");

    /** Find one windows device identifier matching a GUID class and containing a given substring.
     * An exception is thrown if:
     * - there is no device id found
     * - more than one device identifier is found
     *
     * @param[in] guid the GUID class to match
     * @param[in] substring the found devices identifiers have to contain this substring (optional)
     * @return the found device identifier
     * @throw DeviceIdFinder::Exception
     */
    static std::string findOne(const GUID &guid, const std::string &substring = "");

private:
    DeviceIdFinder();

    /** List devices linked to a HDEVINFO info */
    static void getDevices(HDEVINFO devList, std::vector<SP_DEVINFO_DATA> &devices);

    /** Get interface from a device */
    static void getInterfaces(const GUID &guid, HDEVINFO devList, SP_DEVINFO_DATA &device,
                              const std::string &substring, std::set<std::string> &deviceIds);

    /** Get the name of an interface */
    static std::string getInterfaceName(HDEVINFO devList,
                                        SP_INTERFACE_DEVICE_DATA &deviceInterface);
};
}
}
}
