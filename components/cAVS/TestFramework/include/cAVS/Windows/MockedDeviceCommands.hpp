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

#include "cAVS/FirmwareTypes.hpp"
#include "cAVS/Windows/DriverTypes.hpp"
#include "cAVS/Windows/MockedDevice.hpp"
#include <vector>

namespace debug_agent
{
namespace cavs
{
namespace windows
{

/** This class helps to feed a mocked device test vector by adding ioctl commands */
class MockedDeviceCommands final
{
public:
    MockedDeviceCommands(MockedDevice &device) : mDevice(device) {}

    /** Add a get adsp properties command
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] returnedProperties the properties returned by the ioctl.
     *
     * Note: the returnedProperties parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetAdspPropertiesCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        const dsp_fw::AdspProperties &returnedProperties);

    /** Add a get module entries command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] returnedEntries the module entries returned by the ioctl.
     *
     * Note: the returnedEntries parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetModuleEntriesCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        const std::vector<ModuleEntry> &returnedEntries);

    /** Add a get log parameters command
    *
    * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
    * @param[in] returnedStatus the returned driver status
    * @param[in] returnedState the log parameters returned by the ioctl
    *
    * Note: returnedState is unused if
    * - ioctlSuccess is false or
    * - NT_SUCCESS(returnedStatus) returns false
    *
    * @throw Device::Exception
    */
    void addGetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
        const driver::FwLogsState &returnedState);

    /** Add a set log parameters command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedStatus the returned driver status
     * @param[in] expectedState the expected log parameters passed as input buffer to the ioctl
     *
     * Note: the expectedState parameter is alwayse used,
     * even if NT_SUCCESS(returnedStatus) returns false or if ioctlSuccess is false
     *
     * @throw Device::Exception
     */
    void addSetLogParametersCommand(bool ioctlSuccess, NTSTATUS returnedStatus,
        const driver::FwLogsState &expectedState);

private:
    MockedDeviceCommands(const MockedDeviceCommands&) = delete;
    MockedDeviceCommands &operator=(const MockedDeviceCommands&) = delete;

    /** Template method that adds a module access ioctl
     *
     * @tparam FirmwareParameterType the firmware paramerter type, for instance
     * AdspProperties or ModulesInfo */
    template <typename FirmwareParameterType>
    void addGetModuleParameterCommand(dsp_fw::BaseFwParams parameterTypeId,
        const Buffer &returnedParameterContent, bool ioctlSuccess,
        NTSTATUS returnedDriverStatus, dsp_fw::Message::IxcStatus returnedFirmwareStatus);

    MockedDevice &mDevice;
};

}
}
}
