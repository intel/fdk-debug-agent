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

#include "cAVS/Windows/DeviceIdFinder.hpp"
#include "cAVS/Windows/LastError.hpp"
#include "Util/Uuid.hpp"
#include <inttypes.h>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <codecvt>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** Note: this code uses the ugly Win32 SetupDiXXX functions
 * @see https://msdn.microsoft.com/en-us/library/windows/hardware/ff549791(v=vs.85).aspx
 */

/** GUID to string, used for error reporting */
static std::string to_string(const GUID &guid)
{
    util::Uuid uuid;
    uuid.fromOtherUuidType(guid);
    return uuid.toString();
}

/** Create and handle the life cycle of a HDEVINFO handle
 *
 * This class ensures that the handle of type HDEVINFO is properly released
 * in any case using the RAII pattern. */
class DevInfoPtr final
{
public:
    DevInfoPtr(const GUID &guid)
        : mDevInfo(SetupDiGetClassDevs(&guid, 0, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT))
    {
        if (mDevInfo == INVALID_HANDLE_VALUE) {
            throw DeviceIdFinder::Exception("No device interface found for GUID " +
                                            to_string(guid));
        }
    }

    HDEVINFO getHandle() const { return mDevInfo; }

    ~DevInfoPtr() { SetupDiDestroyDeviceInfoList(mDevInfo); }

private:
    DevInfoPtr(const DevInfoPtr &) = delete;
    DevInfoPtr &operator=(const DevInfoPtr &) = delete;

    HDEVINFO mDevInfo;
};

void DeviceIdFinder::getDevices(HDEVINFO devList, std::vector<SP_DEVINFO_DATA> &devices)
{
    /* Enumerating devices */
    for (DWORD index = 0;; index++) {
        SP_DEVINFO_DATA devInfo;
        devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiEnumDeviceInfo(devList, index, &devInfo) != TRUE) {
            DWORD error = GetLastError();
            if (error == ERROR_NO_MORE_ITEMS) {
                break;
            } else {
                throw Exception("Error during device enumeration: " + LastError::get());
            }
        }

        devices.push_back(devInfo);
    }
}

void DeviceIdFinder::getInterfaces(const GUID &guid, HDEVINFO devList, SP_DEVINFO_DATA &device,
                                   const std::string &substring, std::set<std::string> &deviceIds)
{
    /* Enumerating interfaces */
    for (DWORD index = 0;; index++) {
        SP_INTERFACE_DEVICE_DATA ifInfo;
        ifInfo.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

        if (SetupDiEnumDeviceInterfaces(devList, &device, &guid, index, &ifInfo) != TRUE) {
            DWORD error = GetLastError();
            if (error == ERROR_NO_MORE_ITEMS) {
                break;
            } else {
                throw Exception("Error during device interface enumeration: " + LastError::get());
            }
        }

        std::string deviceId = getInterfaceName(devList, ifInfo);

        /* Add the device id if there is no substring or if the interface contains the substring.
         */
        if (substring.length() == 0 || deviceId.find(substring) != std::string::npos) {
            deviceIds.insert(deviceId);
        }
    }
}

std::string DeviceIdFinder::getInterfaceName(HDEVINFO devList,
                                             SP_INTERFACE_DEVICE_DATA &deviceInterface)
{
    /* Getting detail size : first call to SetupDiGetDeviceInterfaceDetail function ...*/
    DWORD requiredSize;
    BOOL ret =
        SetupDiGetDeviceInterfaceDetail(devList, &deviceInterface, NULL, NULL, &requiredSize, NULL);
    assert(ret == FALSE); /* Always return false when getting size... */

    /* Allocating detail data buffer */
    /**
     * @todo remove the usage of operator 'new' once std::make_unique for fixed size array (C++14)
     * is supported by Visual Studio.
     */
    std::unique_ptr<char[]> detailDataBuffer(new char[requiredSize]);
    SP_INTERFACE_DEVICE_DETAIL_DATA *detailData =
        reinterpret_cast<SP_INTERFACE_DEVICE_DETAIL_DATA *>(detailDataBuffer.get());
    detailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    /* Retrieving details : second call to SetupDiGetDeviceInterfaceDetail function ... */
    ret = SetupDiGetDeviceInterfaceDetail(devList, &deviceInterface, detailData, requiredSize, NULL,
                                          NULL);
    if (ret != TRUE) {
        throw Exception("Can not retrieve interface detail content: " + LastError::get());
    }

    return std::string(detailData->DevicePath);
}

void DeviceIdFinder::findAll(const GUID &guid, std::set<std::string> &deviceIds,
                             const std::string &substring)
{
    DevInfoPtr devInfoList(guid);

    std::vector<SP_DEVINFO_DATA> devices;
    getDevices(devInfoList.getHandle(), devices);

    if (devices.empty()) {
        throw Exception("No device found for GUID " + to_string(guid));
    }

    for (auto &device : devices) {
        getInterfaces(guid, devInfoList.getHandle(), device, substring, deviceIds);
    }
}

std::string DeviceIdFinder::findOne(const GUID &guid, const std::string &substring)
{
    std::set<std::string> deviceIds;
    findAll(guid, deviceIds, substring);

    if (deviceIds.size() != 1) {
        throw Exception("More than one device interface found.");
    }

    return *deviceIds.begin();
}
}
}
}
