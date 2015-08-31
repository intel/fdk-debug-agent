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
#include "cAVS/DynamicSizedFirmwareTypes.hpp"
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

    /**
     * Add a get fw config command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] fwConfigTlvList the fw config returned by the ioctl, which is a TLV list.
     *
     * Note: the fwConfigTlvList parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetFwConfigCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        const std::vector<char> &fwConfigTlvList);

    /**
     * Add a get hw config command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] hwConfigTlvList the hw config returned by the ioctl, which is a TLV list.
     *
     * Note: the hwConfigTlvList parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetHwConfigCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        const std::vector<char> &hwConfigTlvList);

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
        const driver::IoctlFwLogsState &returnedState);

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
        const driver::IoctlFwLogsState &expectedState);

    /** Add a get pipeline list command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] maxPplCount the maximum pipeline count
     * @param[in] pipelineIds the pipeline id list returned by the ioctl.
     *
     * Note: the pipelineIds parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetPipelineListCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint32_t maxPplCount,
        const std::vector<uint32_t> &pipelineIds);

    /** Add a get pipeline props command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] pipelineId the id of the requested pipeline
     * @param[in] pipelineProps the returned pipeline properties
     *
     * Note: the pipelineProps parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetPipelinePropsCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint32_t pipelineId,
        const DSPplProps &pipelineProps);

    /** Add a get schedulers info command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] coreId the id of the requested core
     * @param[in] info the returned schedulers info
     *
     * Note: the info parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetSchedulersInfoCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint32_t coreId,
        const DSSchedulersInfo &info);

    /** Add a get gateways command.
     *
     * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
     * @param[in] returnedDriverStatus the returned driver status
     * @param[in] returnedFirmwareStatus the returned firmware status
     * @param[in] gatewayCount the gateway count
     * @param[in] gateways the gateway list returned by the ioctl.
     *
     * Note: the gateways parameter is unused if :
     * - ioctlSuccess is false or
     * - NT_SUCCESS(returnedDriverStatus) returns false or
     * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
     *
     * @throw Device::Exception
     */
    void addGetGatewaysCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint32_t gatewayCount,
        const std::vector<dsp_fw::GatewayProps> &gateways);

    /** Add a get module instance properties command.
    *
    * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
    * @param[in] returnedDriverStatus the returned driver status
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] props the module instance properties returned by the ioctl.
    *
    * Note: the props parameter is unused if :
    * - ioctlSuccess is false or
    * - NT_SUCCESS(returnedDriverStatus) returns false or
    * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
    *
    * @throw Device::Exception
    */
    void addGetModuleInstancePropsCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint16_t moduleId, uint16_t instanceId,
        const DSModuleInstanceProps &props);

    /** Add a set module parameter command.
    *
    * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
    * @param[in] returnedDriverStatus the returned driver status
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] parameterId the id of the requested module parameter
    * @param[in] parameterPayload the parameter payload provided to the ioctl.
    *
    * @throw Device::Exception
    */
    void addSetModuleParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
        const std::vector<uint8_t> &parameterPayload);

    /** Add a get module parameter command.
    *
    * @param[in] ioctlSuccess the returned OS status (when calling DeviceIoControl)
    * @param[in] returnedDriverStatus the returned driver status
    * @param[in] returnedFirmwareStatus the returned firmware status
    * @param[in] moduleId the id of the requested module type
    * @param[in] instanceId the id of the requested module instance
    * @param[in] parameterId the id of the requested module parameter
    * @param[in] parameterPayload the parameter payload provided to the ioctl.
    *
    * Note: the parameterPayload parameter is unused if :
    * - ioctlSuccess is false or
    * - NT_SUCCESS(returnedDriverStatus) returns false or
    * - returnedFirmwareStatus != ADSP_IPC_SUCCESS
    *
    * @throw Device::Exception
    */
    void addGetModuleParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint16_t moduleId, uint16_t instanceId, uint32_t parameterId,
        const std::vector<uint8_t> &parameterPayload);

private:
    MockedDeviceCommands(const MockedDeviceCommands&) = delete;
    MockedDeviceCommands &operator=(const MockedDeviceCommands&) = delete;

    enum class Command
    {
        Get,
        Set
    };

    void addTlvParameterCommand(bool ioctlSuccess, NTSTATUS returnedDriverStatus,
        dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        const std::vector<char> &tlvList, dsp_fw::BaseFwParams parameterId);

    /** Template method that adds a module access ioctl
     *
     * @tparam FirmwareParameterType the firmware paramerter type, for instance
     * AdspProperties or ModulesInfo */
    template <typename FirmwareParameterType>
    void addModuleParameterCommand(Command command, uint32_t parameterTypeId,
        const Buffer &returnedParameterContent, bool ioctlSuccess,
        NTSTATUS returnedDriverStatus, dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint16_t moduleId = baseFirwareModuleId, uint16_t instanceId = baseFirwareInstanceId);

    /** Template method that adds a module access ioctl
     *
     * @tparam FirmwareParameterType the firmware paramerter type, for instance
     * AdspProperties or ModulesInfo */
    template <typename FirmwareParameterType>
    void addModuleParameterCommand(Command command, uint32_t parameterTypeId,
        const Buffer &expectedParameterContent,
        const Buffer &returnedParameterContent, bool ioctlSuccess,
        NTSTATUS returnedDriverStatus, dsp_fw::Message::IxcStatus returnedFirmwareStatus,
        uint16_t moduleId = baseFirwareModuleId, uint16_t instanceId = baseFirwareInstanceId);

    MockedDevice &mDevice;
};

}
}
}
