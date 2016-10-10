/*
 * Copyright (c) 2015, Intel Corporation
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
    struct Exception : std::logic_error
    {
        using std::logic_error::logic_error;
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
