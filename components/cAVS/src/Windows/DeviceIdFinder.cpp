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

#include "cAVS/Windows/DeviceIdFinder.hpp"
#include "cAVS/Windows/LastError.hpp"
#include "Util/Uuid.hpp"
#include <inttypes.h>
#include <memory>
#include <sstream>
#include <iomanip>
#include <assert.h>
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
    DevInfoPtr(const GUID &guid) :
        mDevInfo(SetupDiGetClassDevs(&guid, 0, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT))
    {
        if (mDevInfo == INVALID_HANDLE_VALUE) {
            throw DeviceIdFinder::Exception("No device interface found for GUID " +
                to_string(guid));
        }
    }

    HDEVINFO getHandle() const
    {
        return mDevInfo;
    }

    ~DevInfoPtr()
    {
        SetupDiDestroyDeviceInfoList(mDevInfo);
    }

private:
    DevInfoPtr(const DevInfoPtr &) = delete;
    DevInfoPtr &operator=(const DevInfoPtr &) = delete;

    HDEVINFO mDevInfo;
};


void DeviceIdFinder::getDevices(HDEVINFO devList, std::vector<SP_DEVINFO_DATA> &devices)
{
    /* Enumerating devices */
    for (DWORD index = 0;; index++)
    {
        SP_DEVINFO_DATA devInfo;
        devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiEnumDeviceInfo(devList, index, &devInfo) != TRUE)
        {
            DWORD error = GetLastError();
            if (error == ERROR_NO_MORE_ITEMS) {
                break;
            }
            else {
                throw Exception("Error during device enumeration: " + LastError::get());
            }
        }

        devices.push_back(devInfo);
    }
}

void DeviceIdFinder::getInterfaces(const GUID &guid, HDEVINFO devList,
    SP_DEVINFO_DATA &device, const std::string &substring, std::set<std::string> &deviceIds)
{
    /* Enumerating interfaces */
    for (DWORD index = 0;; index++)
    {
        SP_INTERFACE_DEVICE_DATA ifInfo;
        ifInfo.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);

        if (SetupDiEnumDeviceInterfaces(devList, &device, &guid, index, &ifInfo) != TRUE)
        {
            DWORD error = GetLastError();
            if (error == ERROR_NO_MORE_ITEMS) {
                break;
            }
            else {
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
    BOOL ret = SetupDiGetDeviceInterfaceDetail(devList, &deviceInterface, NULL, NULL,
        &requiredSize, NULL);
    assert(ret == FALSE); /* Always return false when getting size... */

    /* Allocating detail data buffer */
    std::unique_ptr<char[]> detailDataBuffer(new char[requiredSize]);
    SP_INTERFACE_DEVICE_DETAIL_DATA *detailData =
        reinterpret_cast<SP_INTERFACE_DEVICE_DETAIL_DATA*>(detailDataBuffer.get());
    detailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    /* Retrieving details : second call to SetupDiGetDeviceInterfaceDetail function ... */
    ret = SetupDiGetDeviceInterfaceDetail(devList, &deviceInterface, detailData, requiredSize,
        NULL, NULL);
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
